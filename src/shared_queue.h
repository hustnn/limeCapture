#ifndef __SHARED_QUEUE_H
#define __SHARED_QUEUE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <iostream>
#include <string>

using namespace std;

#define QUEUE_LIMIT 10000

typedef struct _addr
{
   int32_t src;
   int32_t dst;
   u_int16_t sPort;
   u_int16_t dPort;
}Addr;

typedef struct _message
{
   int inUsed;
   Addr addr;
   int messages[5]; 
}Message;

struct shared_memory_sync
{
	pthread_mutex_t mutex;
	pthread_cond_t condition;
};

struct shared_memory_data
{
	int head;
	int tail;
	int curlen;
	Message mesQueue[QUEUE_LIMIT];
};

void CreateSharedQueue();
void DeleteSharedQueue();
void OpenSharedQueue();
void CloseSharedQueue();
void EnQueue(Message message);
Message DeQueue();
int ifEmpty();
void SignalQueueHasElement();

#endif
