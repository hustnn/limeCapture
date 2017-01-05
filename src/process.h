#ifndef __PROCESS_H
#define __PROCESS_H

#include <assert.h>
#include <unistd.h>
#include "connection.h"
#include "lime_capture.h"

class Process
{
public:
	char* name;
	char* deviceName;
	int pid;

	u_int32_t inode;
	ConnList* connections;

	Process(u_int32_t _inode,const char* _deviceName,const char* _name = NULL)
	{
	   inode = _inode;
	   if(_name == NULL)
	   	name = _name;
	   else
	   	name = strdup(_name);

	   deviceName = strdup(_deviceName);
	   connections = NULL;
	   pid = 0;
	   uid = 0;
	}

	~Process()
	{
	   free(name);
	   free(deviceName);
	}

	uid_t getUid()
	{
	   return uid;
	}

	void setUid(uid_t _uid)
	{
	   uid = _uid;
	}

private:
	uid_t uid;
};

class ProcList
{
public:
	ProcList* next;

	ProcList(Process* _val,ProcList* _next)
	{
	   assert(_val != NULL);
	   val = _val;
	   next = _next;
	}


	int size();
	Process* getVal()
	{
	   return val;
	}

	Process* getNext()
	{
	   return next;
	}
	
private:
	Process* val;
};

Process* getProcess(Cnnection* connection,char* devicename = NULL);

void process_init ();

#endif