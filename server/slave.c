/*************************************************************************/
/* Copyright (C) Network Programming -USTC, 2012                         */
/*                                                                       */
/*  File Name              :  server/slave.c                             */
/*  Pricipal Author        :  qinpxi                                     */
/*  Subsystem Name         :                                             */
/*  Module Name            :                                             */
/*  Language               :                                             */
/*  Target Environment     :                                             */
/*  Created Time           :  Sun 06 Jan 2013 07:32:55 PM CST            */
/*  Description            :                                             */
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <assert.h>
#include "ServerCtrl.h"
#include "../common/dbProtocol.h"
#include "../common/Socket.h"

#define BACKUP1_DIR     ".//backup1//"
#define BACKUP2_DIR     ".//backup2//"
#define BACKUP_POSTFIX  ".bac"
#define FILE_TO_SEND    "a.db"
#define FILE_MASTER   "a.db"
#define MAX_FILENAME_LEN 128

SlaveList slaves;
int pos;  // This server's position in slaves.

int master_sock;
int listen_sock;
char *addr;
int port;

DBPacketHeader* ExchangePacket(enum CMD cmd, int key, 
        const char *append_str, int len, int sockfd)
{
    static char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader hd;
    hd.cmd = cmd;
    hd.key = key;
    WriteHeader(szBuf, &hd);
    Append(szBuf, append_str, len);
    SendMsg(sockfd, szBuf);
    RecvMsg(sockfd, szBuf);
    return (DBPacketHeader *)szBuf;
}

void AddNewSlave(int pos, SlaveNode sn)
{
    int i;
    for (i=slaves.num-1; i>=pos; i--)
        slaves.nodes[i+1] = slaves.nodes[i];
    slaves.nodes[pos] = sn;
    slaves.num ++;
    slaves.version ++;
}

void HandleMasterRequest()
{
    char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader *phd;
    DBPacketHeader hd;

    RecvMsg(master_sock, szBuf);
    phd = (DBPacketHeader *)szBuf;

    switch (phd->cmd)
    {
        case NEW_SLAVE:
            {
                AddNewSlave(phd->key, *(SlaveNode *)GetAppend(phd));
                printslaves(slaves);
                hd.cmd = NEW_SLAVE_R;
                WriteHeader(szBuf, &hd);
                SendMsg(master_sock, szBuf);
                break;
            }
        case RM_SLAVE:
            {
                // RemoveSlave();
                // if I am the predecessor of the removed slave
                //    store backup1's data into origin data
                //    replace backup1 by backup2
                //    Backup data for 2nd-successor
                // else if I am the 2nd-predecessor of the removed slave
                //    Merge backup2 into backup1
                //    Backup data for 2nd-successor
            }
        default:
            {
            }
    }

}

// first send file name (Append in dbProtocol) and 
// the size of the file (key in DBPacketHeader), 
// then the file content
int SendFile(int sockfd, const char *filename, enum CMD rescmd)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Open file:%s failed.\n", filename);
        return -1;
    }
    printf("Start sending file:%s\n", filename);
    fseek(fp, 0, SEEK_END);
    int flen = ftell(fp);

    DBPacketHeader *phd = 
        ExchangePacket(rescmd, flen, filename, strlen(filename)+1, sockfd);
    if (phd->cmd != FILETRANS_R)
    {
        fprintf(stderr, "File info transfer failed.\n");
        return -1;
    }

    fseek(fp, 0, SEEK_SET);
    int finish_size = 0;
    while (finish_size < flen)
    {
        char szBuf[MAX_BUF_LEN] = "\0";
        int len =  fread(szBuf, sizeof(char), APPENDSIZE, fp);
        phd = ExchangePacket(FILETRANS, len, szBuf, len, sockfd);
        if (phd->cmd != FILETRANS_R)
        {
            fprintf(stderr, "File transfer failed.\n");
            return -1;
        }
        finish_size += len;
    }
    printf("Sending file complete.\n\n");

    return 0;
}

// k determines whether the file is origin file or backup file
int RecvFile(int sockfd, char *filename, int len)
{
    // open file 
    printf("Start recving file:%s\n", filename);
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
        printf("open file failed.\n");
    int total_size = len;

    // transfer
    int finish_size = 0;
    while (finish_size < total_size)
    {
        DBPacketHeader *phd = ExchangePacket(FILETRANS_R, 0, NULL, 0, sockfd);
        int write_len = fwrite(GetAppend(phd), sizeof(char), phd->key, fp);
        if (write_len < phd->key)
        {
            fprintf(stderr, "Write data to file failed.\n");
            return -1;
        }
        finish_size += phd->key;
    }
    char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader hd;
    hd.cmd = FILETRANS_R;
    WriteHeader(szBuf, &hd);
    SendMsg(sockfd, szBuf);

    fclose(fp);
    printf("Recving file complete.\n\n");

    return 0;
}

void ReplaceBackup2ByBackup1()
{
    char cpcmd[128] = "cp ";
    strcat(cpcmd, BACKUP1_DIR);
    strcat(cpcmd, "* ");
    strcat(cpcmd, BACKUP2_DIR);
    if (0 != system(cpcmd))
        printf("Replace backup2 by backup1 failed.\n");
    strcpy(cpcmd, "rm ");
    strcat(cpcmd, BACKUP1_DIR);
    strcat(cpcmd, "*");
    if (0 != system(cpcmd))
        printf("Remove files backup1/* failed.\n");
}

void SaveTheSendedFileInBackup1()
{
    char cpcmd[128] = {"cp "};
    strcat(cpcmd, FILE_TO_SEND);
    strcat(cpcmd, " ");
    strcat(cpcmd, BACKUP1_DIR);
    strcat(cpcmd, FILE_TO_SEND);
    strcat(cpcmd, BACKUP_POSTFIX);
    if (0 != system(cpcmd))
        printf("Backup for successor failed.\n");
}

void RemoveBackup2()
{
    char rmcmd[128] = "rm ";
    strcat(rmcmd, BACKUP2_DIR);
    strcat(rmcmd, "*");
    if (0 != system(rmcmd))
        printf("Remove backup2/* failed.\n");
}

void HandleSlaveRequest()
{
    int accept_sock;
    int ip;
    char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader *phd;

    ServiceStart(listen_sock, &accept_sock, &ip);
    RecvMsg(accept_sock, szBuf);
    phd = (DBPacketHeader *)szBuf;

    switch (phd->cmd)
    {
        case CLIP_DATA:
            {
                // replace backup2 by backup1
                ReplaceBackup2ByBackup1();
                // split the data the new slave uses into FILE_TO_SEND
                printf("clip after %d.\n", phd->key);
                // SplitByKey(filetosplit, phd->key, FILE_TO_SEND);
                // send FILE_TO_SEND
                SendFile(accept_sock, FILE_TO_SEND, CLIP_DATA_R);
                // save the sended file in backup1/
                SaveTheSendedFileInBackup1();
                // Finish.
                ServiceStop(accept_sock);
                break;
            }
        case BACKUP:
            {
                // send all of the data (not backup data, just origin data)
                // (BACKUP_R)
                SendFile(accept_sock, FILE_MASTER, BACKUP_R);
                ServiceStop(accept_sock);
                break;
            }
        case UPDATE_BACKUP2:
            {
                char szBuf[MAX_BUF_LEN] = "\0";
                DBPacketHeader hd;
                hd.cmd = UPDATE_BACKUP2_R;
                WriteHeader(szBuf, &hd);
                SendMsg(accept_sock, szBuf);
                // delete backup2
                RemoveBackup2();
                // split backup1 by key(phd->key) 
                break;
            }
        default:
            {
            }
    }
}

void HandleCtrlRequest()
{
    int epollid;
    struct epoll_event event;

    epollid = epoll_create(1024);
    event.data.fd = listen_sock;
    event.events = EPOLLIN | EPOLLRDHUP;
    epoll_ctl(epollid, EPOLL_CTL_ADD, listen_sock, &event);
    event.data.fd = master_sock;
    event.events = EPOLLIN | EPOLLRDHUP;
    epoll_ctl(epollid, EPOLL_CTL_ADD, master_sock, &event);

    while (1)
    {
        epoll_wait(epollid, &event, 1, -1);
        if (event.data.fd == listen_sock)
        {
            printf("listen_sock.\n");
            HandleSlaveRequest();
        }
        else if ((event.events & EPOLLIN) && (event.events & EPOLLRDHUP))
        {
            printf("Remote service shut down.\n");
            CloseRemoteService(event.data.fd);
        }
        else if (event.data.fd == master_sock)
        {
            printf("master_sock.\n");
            HandleMasterRequest(master_sock);
        }
    }
}

void RegisterAndLoadSlaves(int master_sock)
{
    DBPacketHeader *phd = ExchangePacket(ADD_SLAVE, port, NULL, 0, master_sock);
    assert(phd->cmd == ADD_SLAVE_R);
    pos = phd->key;
    slaves = *(SlaveList *)GetAppend(phd);
    printslaves(slaves);
}

char *GetFileName(int k, char *filename)
{
    static char filepath[][MAX_FILENAME_LEN] = {
        {"\0"}, {BACKUP1_DIR}, {BACKUP2_DIR}
    };
    strcat(filepath[k], filename);
    if (k != 0)
        strcat(filepath[k], BACKUP_POSTFIX);

    return filepath[k];
}

int ClipDataFromPre()
{
    int prepos = (pos - 1 + slaves.num) % slaves.num;
    SlaveNode pre_slave = slaves.nodes[prepos];

    if (-1 == OpenRemoteService2(&pre_slave.sock, 
                pre_slave.ip, pre_slave.port))
    {
        fprintf(stderr, "Connet to slave:%d failed.\n", prepos);
        return -1;
    }
    printf("Connect to slave: %d\n", prepos);

    DBPacketHeader *phd = ExchangePacket(CLIP_DATA, 
            slaves.nodes[pos].key, NULL, 0, pre_slave.sock);
    assert(phd->cmd == CLIP_DATA_R);

    // choose file according to k
    char *filename = GetFileName(0, GetAppend(phd));

    return RecvFile(pre_slave.sock, filename, phd->key);
}

int BackupForOtherSlave(int k)
{
    int hispos = (pos + k) % slaves.num;
    SlaveNode slave = slaves.nodes[hispos];

    if (-1 == OpenRemoteService2(&slave.sock, slave.ip, slave.port))
    {
        fprintf(stderr, "Connet to slave:%d failed.\n", hispos);
        return -1;
    }
    printf("Connect to slave: %d\n", hispos);
    
    DBPacketHeader *phd = ExchangePacket(BACKUP, 0, NULL, 0, slave.sock);
    assert(phd->cmd == BACKUP_R);
    
    char *filename = GetFileName(k, GetAppend(phd));

    return RecvFile(slave.sock, filename, phd->key);
}

void Notify2ndPreUpdateBackup2()
{
    int hispos = (pos - 2 + slaves.num) % slaves.num;
    SlaveNode pre2_slave = slaves.nodes[hispos];

    if (-1 == OpenRemoteService2(&pre2_slave.sock, pre2_slave.ip, pre2_slave.port))
    {
        fprintf(stderr, "Connet to slave:%d failed.\n", hispos);
        return ;
    }
    printf("Connect to slave: %d\n", hispos);

    DBPacketHeader *phd = ExchangePacket(UPDATE_BACKUP2,
            0, NULL, 0, pre2_slave.sock);
    assert(phd->cmd == UPDATE_BACKUP2_R);
}

int main(int argc, char *argv[])
{
    pthread_t ctrl_thread_id;
    if (argc != 3)
    {
        printf("Address and port is needed.\n");
        exit(-1);
    }

    InitializeService(&listen_sock, NULL, 0);
    port = GetPort(listen_sock);

    if (-1 == OpenRemoteService(&master_sock,  argv[1],  atoi(argv[2])))
        return -1;
    RegisterAndLoadSlaves(master_sock);

    /*if (0 != system("mkdir backup1"))
        printf("mkdir backup1 failed.\n");
    if (0 != system("mkdir backup2"))
        printf("mkdir backup2 failed.\n");*/
        
    if (slaves.num > 1)
    {
        if (-1 == ClipDataFromPre())
            return -1;
        BackupForOtherSlave(1);
    }
    if (slaves.num > 2)
    {
        Notify2ndPreUpdateBackup2();
        BackupForOtherSlave(2);
    }

    pthread_create(&ctrl_thread_id, NULL, (void*)HandleCtrlRequest, NULL);

    while (1)
        ;  // DB operating
    return 0;
}
