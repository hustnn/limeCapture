#include "shared_queue.h"
#include <stdio.h>
#include <string.h>

typedef void* ADDR;

static ADDR addr_sync;
static ADDR addr_data;

static const string shm_name_sync("/sync");
static const string shm_name_data("/data");


static struct shared_memory_sync* p_sync;
static struct shared_memory_data* p_data;

void CreateSharedQueue()
{
	int fd_sync = shm_open(shm_name_sync.c_str(),O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
	ftruncate(fd_sync,sizeof(shared_memory_sync));
	addr_sync = mmap(0,sizeof(shared_memory_sync),PROT_READ|PROT_WRITE,MAP_SHARED,fd_sync,0);
	p_sync = static_cast<shared_memory_sync*>(addr_sync);
	
	pthread_condattr_t cond_attr;
	pthread_condattr_init(&cond_attr);
	pthread_condattr_setpshared(&cond_attr,PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&(p_sync->condition),&cond_attr);
	pthread_condattr_destroy(&cond_attr);

	pthread_mutexattr_t m_attr;
	pthread_mutexattr_init(&m_attr);
	pthread_mutexattr_setpshared(&m_attr,PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&(p_sync->mutex),&m_attr);
	pthread_mutexattr_destroy(&m_attr);

	int fd_data = shm_open(shm_name_data.c_str(),O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
	ftruncate(fd_data,sizeof(shared_memory_data));
	addr_data = mmap(0,sizeof(shared_memory_data),PROT_READ|PROT_WRITE,MAP_SHARED,fd_data,0);
	p_data = static_cast<shared_memory_data*>(addr_data);

	memset(p_data,0,sizeof(shared_memory_data));
	p_data->head = 0;
	p_data->tail = -1;
	p_data->curlen = 0;
}



void DeleteSharedQueue()
{
	munmap(addr_sync,sizeof(shared_memory_sync));
	shm_unlink(shm_name_sync.c_str());
	munmap(addr_data,sizeof(shared_memory_data));
	shm_unlink(shm_name_data.c_str());
}


void OpenSharedQueue()
{
	int fd_sync = shm_open(shm_name_sync.c_str(),O_RDWR,S_IRUSR|S_IWUSR);
	addr_sync = mmap(0,sizeof(shared_memory_sync),PROT_READ|PROT_WRITE,MAP_SHARED,fd_sync,0);
	p_sync = static_cast<shared_memory_sync*>(addr_sync);

	int fd_data = shm_open(shm_name_data.c_str(),O_RDWR,S_IRUSR|S_IWUSR);
	addr_data = mmap(0,sizeof(shared_memory_data),PROT_READ|PROT_WRITE,MAP_SHARED,fd_data,0);
	p_data = static_cast<shared_memory_data*>(addr_data);
}

void CloseSharedQueue()
{
	munmap(addr_sync,sizeof(shared_memory_sync));
	munmap(addr_data,sizeof(shared_memory_data));
}

void EnQueue(Message message)
{
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~EnQueue\n");
	printf("head is %d,tail is %d,curlen is %d\n",p_data->head,p_data->tail,p_data->curlen);
	pthread_mutex_lock(&(p_sync->mutex));
	p_data->tail += 1;
	p_data->tail %= QUEUE_LIMIT;
	
	if(p_data->mesQueue[p_data->tail].inUsed == 0)
   	{
		p_data->curlen += 1;
	}
	else if(p_data->tail == p_data->head)
	{
		p_data->head += 1;
		p_data->head %= QUEUE_LIMIT;
	}

	pthread_mutex_unlock(&(p_sync->mutex));
	p_data->mesQueue[p_data->tail] = message;
	
	printf("head is %d,tail is %d,curlen is %d\n",p_data->head,p_data->tail,p_data->curlen);
}

Message DeQueue()
{
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~DeQueue\n");
	printf("head is %d,tail is %d,curlen is %d\n",p_data->head,p_data->tail,p_data->curlen);

	Message mes;
	pthread_mutex_lock(&(p_sync->mutex));
	p_data->head += 1;
	p_data->head %= QUEUE_LIMIT;
	if(p_data->curlen > 0)
		p_data->curlen -= 1;
	pthread_mutex_unlock(&(p_sync->mutex));
	mes = p_data->mesQueue[p_data->head];
	memset(&(p_data->mesQueue[p_data->head]),0,sizeof(Message));

	printf("head is %d,tail is %d,curlen is %d\n",p_data->head,p_data->tail,p_data->curlen);

	return mes;
}

int ifEmpty()
{
	int ret = 0;
	pthread_mutex_lock(&(p_sync->mutex));
	if(p_data->curlen == 0)
		ret = 1;
	else
		ret = 0;
	pthread_mutex_unlock(&(p_sync->mutex));
	return ret;	
}

void SignalQueueHasElement()
{
	pthread_cond_signal(&(p_sync->condition));
}

