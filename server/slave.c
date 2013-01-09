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

SlaveList slaves;
int pos;  // This server's position in slaves.

int master_sock;
int listen_sock;
char *addr;
int port;

void Send(enum CMD cmd, int key, char *buf)
{
}

void AddNewSlave(int pos, SlaveNode sn)
{
    int i;
    printf("slaves: num:%d pos:%d\n", slaves.num, pos);
    for (i=slaves.num-1; i>=pos; i--)
        slaves.nodes[i+1] = slaves.nodes[i];
    slaves.nodes[pos] = sn;
    slaves.num ++;
    slaves.version ++;
}

void HandleMasterRequest(int master_sock)
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
void SendFile(int accept_sock)
{
    char filename[] = "a.db";
    FILE *fp = fopen(filename, "r");
    fseek(fp, 0, SEEK_END);
    int flen = ftell(fp);
    char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader hd;
    hd.cmd = CLIP_DATA_R;
    hd.key = flen;
    WriteHeader(szBuf, &hd);
    Append(szBuf, filename, sizeof(filename));
    printf("length of filename:%d\n", sizeof(filename));
    SendMsg(accept_sock, szBuf);

    RecvMsg(accept_sock, szBuf);
    DBPacketHeader *phd = (DBPacketHeader *)szBuf;
    if (phd->cmd != FILETRANS_R)
    {
        fprintf(stderr, "File info transfer failed.\n");
        return;
    }

    fseek(fp, 0, SEEK_SET);
    int finish_size = 0;
    while (finish_size < flen)
    {
        hd.cmd = FILETRANS;
        hd.key = fread(GetAppend(phd), sizeof(char), 
                MAX_BUF_LEN - sizeof(DBPacketHeader), fp);
        WriteHeader(szBuf, &hd);
        SendMsg(accept_sock, szBuf);

        RecvMsg(accept_sock, szBuf);
        if (phd->cmd != FILETRANS_R)
        {
            fprintf(stderr, "File transfer failed.\n");
            return;
        }
        finish_size += hd.key;
    }
}

void HandleSlaveRequest()
{
    int accept_sock;
    int ip;
    char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader *phd;
    DBPacketHeader hd;

    ServiceStart(listen_sock, &accept_sock, &ip);
    RecvMsg(accept_sock, szBuf);
    phd = (DBPacketHeader *)szBuf;

    switch (phd->cmd)
    {
        case CLIP_DATA:
            {
                // replace backup2 by backup1
                // find the key the new slave uses
                // send all of the data after key (CLIP_DATA_R)
                // store the sended data into backup1
                SendFile(accept_sock);
                ServiceStop(accept_sock);
            }
        case BACKUP:
            {
                // send all of the data (not backup data, just origin data)
                // (BACKUP_R)
            }
        case UPDATE_BACKUP:
            {
                // delete backup2
                // split backup2 from backup1
            }
        default:
            {
            }
    }
}

void HandleCtrlRequest(int master_sock)
{
    int epollid;
    struct epoll_event event;

    epollid = epoll_create(1024);
    event.data.fd = listen_sock;
    event.events = EPOLLIN | EPOLLRDHUP;
    epoll_ctl(epollid, EPOLL_CTL_ADD, listen_sock, &event);
    event.data.fd = master_sock;
    event.events = EPOLLIN;
    epoll_ctl(epollid, EPOLL_CTL_ADD, master_sock, &event);

    while (1)
    {
        epoll_wait(epollid, &event, 1, -1);
        if (event.data.fd == listen_sock)
        {
            HandleSlaveRequest();
        }
        else if (event.data.fd == master_sock)
        {
            HandleMasterRequest(master_sock);
        }
    }
}

void RegisterAndLoadSlaves(int master_sock)
{
    char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader hd;
    hd.cmd = ADD_SLAVE;
    hd.key = port;
    WriteHeader(szBuf, &hd);
    SendMsg(master_sock, szBuf);

    RecvMsg(master_sock, szBuf);
    DBPacketHeader *phd = (DBPacketHeader *)szBuf;
    assert(phd->cmd == ADD_SLAVE_R);
    pos = phd->key;
    slaves = *(SlaveList *)GetAppend(phd);
    printslaves(slaves);
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

    char szBuf[MAX_BUF_LEN] = "\0";
    DBPacketHeader hd;
    hd.cmd = CLIP_DATA;
    WriteHeader(szBuf, &hd);
    SendMsg(pre_slave.sock, szBuf);
    RecvMsg(pre_slave.sock, szBuf);
    DBPacketHeader *phd = (DBPacketHeader *)szBuf;
    assert(phd->cmd == CLIP_DATA_R);

    FILE *fp = fopen(GetAppend(phd), "w");
    int total_size = phd->key;
    hd.cmd = FILETRANS_R;
    WriteHeader(szBuf, &hd);
    SendMsg(pre_slave.sock, szBuf);

    int finish_size = 0;
    while (finish_size < total_size)
    {
        RecvMsg(pre_slave.sock, szBuf);
        phd = (DBPacketHeader *)szBuf;
        // write GetAppend(phd) into file;
        int write_len = fwrite(GetAppend(phd), sizeof(char), phd->key, fp);
        if (write_len < phd->key)
        {
            fprintf(stderr, "Write data to file failed.\n");
            return -1;
        }
        finish_size += phd->key;

        hd.cmd = FILETRANS_R;
        WriteHeader(szBuf, &hd);
        SendMsg(pre_slave.sock, szBuf);
    } 
    fclose(fp);

    return 0;
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
    if (slaves.num != 1)
    {
        // Clip data from predecessor
        if (-1 == ClipDataFromPre())
            return -1;
        // Notify 2nd-predecessor UPDATE_BACKUP
        // Backup data for successor and 2nd-successor
    }

    pthread_create(&ctrl_thread_id, NULL, (void*)HandleCtrlRequest, (void*)master_sock);

    while (1)
        ;  // DB operating
    return 0;
}
