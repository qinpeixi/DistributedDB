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
#include <signal.h>
#include <assert.h>
#include "ServerCtrl.h"
#include "slave.h"
#include "../common/dbProtocol.h"
#include "../common/Socket.h"
#include "../common/Database.h"

#define BACKUP1_DIR     "backup1"
#define BACKUP2_DIR     "backup2"
#define BACKUP_POSTFIX  ".bac"
//#define FILE_TO_SEND    "a.db~"
//#define FILE_MASTER     "a.db"
#define MAX_FILENAME_LEN 128

char file_tmp[MAX_FILENAME_LEN] = "a.db~";
char file_master[MAX_FILENAME_LEN] = "a.db";

SlaveList slaves;
int pos;  // This server's position in slaves.

int master_sock;
//int listen_sock;
char *addr;
int port;
pthread_t ctrl_thread_id;

unsigned GetVersion()
{
    return slaves.version;
}

void SendSlaveList(int sockfd)
{
    DBPacketHeader hd;
    hd.cmd = UPDATE_VERSION;
    hd.version = slaves.version;
    char szBuf[MAX_BUF_LEN] = "\0";
    WriteHeader(szBuf, &hd);
    Append(szBuf, (char *)&slaves, sizeof(slaves));
    SendMsg(sockfd, szBuf);
}

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
    printf("add node %d.\n", pos);
    int i;
    for (i=slaves.num-1; i>=pos; i--)
        slaves.nodes[i+1] = slaves.nodes[i];
    slaves.nodes[pos] = sn;
    slaves.num ++;
    slaves.version ++;
}

void RemoveSlave(int pos)
{
    printf("remove node %d.\n", pos);
    int i;
    for (i=pos; i<slaves.num-1; i++)
        slaves.nodes[i] = slaves.nodes[i+1];
    slaves.num --;
    slaves.version ++;
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
    printf("Sending file complete.\n");

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
    printf("Recving file complete.\n");

    return 0;
}

char* ExeShellCmd(const char *cmd, const char *srcdir, const char *srcfname,
        const char *destdir, const char *destfname)
{
    static char fullcmd[2 * MAX_FILENAME_LEN];
    strcpy(fullcmd, cmd);
    strcat(fullcmd, " ");
    strcat(fullcmd, srcdir);
    if (strlen(srcdir) != 0)
        strcat(fullcmd, "//");
    strcat(fullcmd, srcfname);
    if (strlen(srcdir) != 0 && strlen(srcfname) != 0)  // must be backup1 or backup2
        strcat(fullcmd, BACKUP_POSTFIX);
    strcat(fullcmd, " ");
    strcat(fullcmd, destdir);
    if (strlen(destdir) != 0)
        strcat(fullcmd, "//");
    strcat(fullcmd, destfname);
    if (strlen(destdir) != 0 && strlen(destfname) != 0) 
        strcat(fullcmd, BACKUP_POSTFIX);
    if (strlen(cmd) != 0 && system(fullcmd) != 0)
        printf("Execute shell command failed:%s\n", fullcmd);

    printf("Execute: %s\n", fullcmd);
    return fullcmd;
}

char *GetFileName(int k, char *filename)
{
    static char filepath[][MAX_FILENAME_LEN] = {
        {"\0"}, {BACKUP1_DIR}, {BACKUP2_DIR}
    };
    if (k > 0)
        strcat(filepath[k], "//");
    strcat(filepath[k], filename);
    if (k != 0)
        strcat(filepath[k], BACKUP_POSTFIX);

    return filepath[k];
}

int CommunicateToSlave(int k, enum CMD cmd, int key, enum CMD rescmd)
{
    int hispos = (pos + k + slaves.num) % slaves.num;
    if (hispos == pos)
        return 0;
    SlaveNode slave = slaves.nodes[hispos];

    if (-1 == OpenRemoteService2(&slave.sock, slave.ip, slave.port))
    {
        fprintf(stderr, "Connet to slave:%d failed.\n", hispos);
        return -1;
    }
    printf("Connect to slave: %d\n", hispos);
    
    DBPacketHeader *phd = ExchangePacket(cmd, key, NULL, 0, slave.sock);
    assert(phd->cmd == rescmd);
    
    int res = -1;
    if (k == -2)
        res = 0;
    else 
    {
        k = (k == -1) ? 0 : k;
        char *filename = GetFileName(k, GetAppend(phd));
        res = RecvFile(slave.sock, filename, phd->key);
    }
    
    CloseRemoteService(slave.sock);
    return res;
}

int ClipDataFromPre()
{
    return CommunicateToSlave(-1, CLIP_DATA, slaves.nodes[pos].key,
            CLIP_DATA_R);
}

int BackupForOtherSlave(int k)
{
    return CommunicateToSlave(k, BACKUP, 0, BACKUP_R);
}

int Notify2ndPreUpdateBackup2()
{
    return CommunicateToSlave(-2, UPDATE_BACKUP2, 0, UPDATE_BACKUP2_R);
}

void HandleSlaveRequest(int accept_sock, char *szBuf)
{
    DBPacketHeader *phd = (DBPacketHeader *)szBuf;

    switch (phd->cmd)
    {
        case CLIP_DATA:
            {
                printf("\nCLIP_DATA\n");
                ExeShellCmd("mv", BACKUP1_DIR, "*", BACKUP2_DIR, "");
                // split the data the new slave uses into file_tmp
                int lower_b = slaves.nodes[(pos + 1)%slaves.num].key;
                int upper_b = slaves.nodes[(pos + 2)%slaves.num].key;
                printf("clip after %d.\n", phd->key);
                ExeShellCmd("mv", "", file_master, "", file_tmp);
                SplitByKey(file_tmp, lower_b, file_master, upper_b);
                SendFile(accept_sock, file_master, CLIP_DATA_R);
                ExeShellCmd("cp", "", file_master, BACKUP1_DIR, file_master);
                ExeShellCmd("mv", "", file_tmp, "", file_master);
                ServiceStop(accept_sock);
                break;
            }
        case BACKUP:
            {
                printf("\nBAKCUP\n");
                SendFile(accept_sock, file_master, BACKUP_R);
                ServiceStop(accept_sock);
                break;
            }
        case UPDATE_BACKUP2:
            {
                printf("\nUPDATE_BACKUP2\n");
                char szBuf[MAX_BUF_LEN] = "\0";
                // delete backup2
                ExeShellCmd("rm", BACKUP2_DIR, "*", "", "");
                // split backup1 by key(phd->key) 
                char fname1[MAX_FILENAME_LEN];
                char fname2[MAX_FILENAME_LEN];
                strcpy(fname1, BACKUP1_DIR);
                strcpy(fname2, BACKUP2_DIR);
                strcat(fname1, "/");
                strcat(fname1, file_master);
                strcat(fname1, BACKUP_POSTFIX);
                strcat(fname2, "/");
                strcat(fname2, file_master);
                strcat(fname2, BACKUP_POSTFIX);
                int lower_b = slaves.nodes[(pos + 2)%slaves.num].key;
                int upper_b = slaves.nodes[(pos + 3)%slaves.num].key;
                SplitByKey(fname1, lower_b, fname2, upper_b);

                DBPacketHeader hd;
                hd.cmd = UPDATE_BACKUP2_R;
                WriteHeader(szBuf, &hd);
                SendMsg(accept_sock, szBuf);
                break;
            }
        default:
            {
            }
    }
    ServiceStop(accept_sock);
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
                printf("\nNEW_SLAVE\n");
                AddNewSlave(phd->key, *(SlaveNode *)GetAppend(phd));
                printslaves(slaves);
                if (phd->key <= pos)
                    pos = (pos + 1) % slaves.num;
                printf("my no. : %d.\n", pos);
                hd.cmd = NEW_SLAVE_R;
                WriteHeader(szBuf, &hd);
                SendMsg(master_sock, szBuf);
                break;
            }
        case RM_SLAVE:
            {
                printf("\nRM_SLAVE\n");
                int rmpos = phd->key;
                RemoveSlave(rmpos);
                printslaves(slaves);
                pos = (pos - 1 + slaves.num) % slaves.num;
                printf("my no. : %d.\n", pos);

                hd.cmd = RM_SLAVE_R;
                WriteHeader(szBuf, &hd);
                SendMsg(master_sock, szBuf);

                // if I am the predecessor of the removed slave
                if (pos == ((rmpos - 1 + slaves.num) % slaves.num))
                {
                    printf("if I am the predecessor of the removed slave\n");
                //    merge backup1's data into origin data
                    char filename[100];
                    strcpy(filename, BACKUP1_DIR);
                    strcat(filename, "//");
                    strcat(filename, file_master);
                    strcat(filename, BACKUP_POSTFIX);
                    Merge2Files(filename, file_master);
                //    replace backup1 by backup2
                    ExeShellCmd("mv", BACKUP2_DIR, "*", BACKUP1_DIR, "");
                //    Backup data for 2nd-successor
                    BackupForOtherSlave(2);
                }
                // else if I am the 2nd-predecessor of the removed slave
                else if (pos == ((rmpos - 2 + slaves.num) % slaves.num))
                {
                    printf("else if I am the 2nd-predecessor of the removed slave\n");
                //    Merge backup2 into backup1
                    char filename1[100];
                    char filename2[100];
                    strcpy(filename1, BACKUP1_DIR);
                    strcat(filename1, "//");
                    strcpy(filename2, BACKUP2_DIR);
                    strcat(filename2, "//");
                    strcat(filename1, file_master);
                    strcat(filename2, file_master);
                    strcat(filename1, BACKUP_POSTFIX);
                    strcat(filename2, BACKUP_POSTFIX);
                    Merge2Files(filename2, filename1);
                //    Backup data for 2nd-successor
                    ExeShellCmd("rm", BACKUP2_DIR, "*", "", "");
                    BackupForOtherSlave(2);
                }
                break;
            }
        default:
            {
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

void ShutDownSlave(int a)
{
    printf("Shut down.\n");
    pthread_cancel(ctrl_thread_id);
    DBPacketHeader *phd = ExchangePacket(DEL_SLAVE, pos, NULL, 0, master_sock);
    assert(phd->cmd == DEL_SLAVE_R);
    ShutdownService(listen_sock);
    CloseRemoteService(master_sock);
    exit(0);
}

int InitialSlave(char *master_addr, int master_port)
{
    port = GetPort(listen_sock);

    if (-1 == OpenRemoteService(&master_sock,  master_addr,  master_port))
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

    return 0;
}
