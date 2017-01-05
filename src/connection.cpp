#include "connection.h"
#include "lime_capture.h"
#include "util.h"
#include <iostream>
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include <mysql.h>

#include "connection_hashMap.h"

static int max_fds;
static int next_slot;
static int current_time;
static ConnList** fd_ring;
static ConnList* connection_hash[HASH_SIZE];

void initCoonState()
{
   int i;
   for(i = 0; i < HASH_SIZE; i++)
      connection_hash[i] = NULL;
}

void* Connection::operator new(size_t size)
{
   hashMapElement* elem = CursorAlloc();
   return (void*)(&(elem->data));
}

void Connection::operator delete(void* p)
{
   Connection* c = (Connection*)p;
   CursorFree(c->owner);
}

Connection::Connection()
{

}


Connection::Connection(Packet* packet)
{
   int32_t key = packet->hashPacket();
   owner->key = key;
   //HashMapInsert(key,owner);
   flags = 0;
   iseq = packet->seq;
   curState = stat_empty;
   iniPacket = new Packet(*packet);
   prePacket = NULL;

   connTimestamp = packet->time;
   //currMessCount = 0;
   currMessCount = -1;
   memset(messages,0,sizeof(int) * MESS_SIZE);
   resetTcpReassembly();

   HashMapInsert(owner->key,owner);
}

/*
Connection::Connection(Packet * packet)
{
   assert(packet != NULL);
   ConnList* ptr;
   u_int32_t index = packet->hashPacket();
   printf("the packet hash value is %u\n",index);
   ptr = connection_hash[index];
   connection_hash[index] = new ConnList(this,ptr);
   fp = NULL;
   hashValue = index;
   //currMessCount = 0;
   flags = 0;
   iseq = packet->seq;
   curState = stat_empty;*/

  /* if(packet->Outgoing())
   {
      synDirection = dir_outgoing;
      refpacket = new Packet(*packet);
   }
   else
   {
      synDirection = dir_incoming;
      refpacket = packet->newInverted();
   }*/

   /*iniPacket = new Packet(*packet);
   //latDirection = synDirection;

   connTimestamp = packet->time;
   lastAccess = packet->time.tv_sec;

   currMessCount = 0;
   memset(messages,0,sizeof(int) * MESS_SIZE);
   prePacket = NULL;
   resetTcpReassembly();
   
}*/

void Connection::resetTcpReassembly()
{
   int i;
   for(i = 0; i < 2; i++)
   {
      seq[i] = 0;
      ack[i] = 0;
      ipAddr[i] = 0;
      portAddr[i] = 0;

      frags[i] = new PackList();
    }
}

Connection::~Connection()
{
   int i;
   delete iniPacket;
   delete prePacket;
   for(i = 0;i < 2;i++)
   {
      if(frags[i] != NULL)
         delete frags[i];
   }
   
   HashMapRemove(owner);
}

/*
Connection::~Connection()
{
	int i;
	//delete(refpacket);
	delete(iniPacket);
	delete(prePacket);
	for(i = 0; i < 2; i++)
	{
		if(frags[i] != NULL)
			delete frags[i];
	}

	ConnList* curr_conn = connection_hash[hashValue];
	ConnList* prev_conn = NULL;
	while(curr_conn != NULL)
	{
		if(curr_conn->val == this)
		{
			ConnList* todelete = curr_conn;
			curr_conn = curr_conn->next;
			if(prev_conn == NULL)
				connection_hash[hashValue] = curr_conn;
			else
				prev_conn->next = curr_conn;

			delete todelete;
		}
		else
		{
		   prev_conn = curr_conn;
		   curr_conn = curr_conn->next;
		}
	}
}*/

/*
int Connection::closeFile()
{
   if(fp == NULL)
   	return 0;
   fclose(fp);
   fp = NULL;
   return 0;
}*/

/*

FILE* Connection::attemptFopen(char* filename)
{
   if(IS_SET(flags, CONN_FILE_EXISTS))
   {
      debug_real("reopenning opened file %s",filename);
      fp = fopen(filename,"r+");
   }
   else
   {
      debug_real("openning new output file %s",filename);
      fp = fopen(filename,"w");
   }

   return fp;
}*/


/*
FILE* Connection::openFile()
{
   char* filename = iniPacket->packetFileName();
   int done = 0;
   
   if(fp)
   {
      debug_real("trying to open already open file");
      return fp;
   }

   attemptFopen(filename);

   if(fp == NULL)
   {
      SET_BIT(flags,CONN_FINISHED);
      return NULL;
   }

   SET_BIT(flags,CONN_FILE_EXISTS);
   return fp;
}*/

Connection* findConnection(Packet* packet)
{
   hashMapElement* elem;
   int ret;
   ret = HashMapGet(packet,&elem);
   if(ret == MAP_OK)
      return &(elem->data);
   
   Packet* invertedPacket = packet->newInverted();
   ret = HashMapGet(invertedPacket,&elem);

   delete invertedPacket;

   if(ret == MAP_OK)
      return &(elem->data);
   else
      return NULL;
}

/*
Connection* findConnection(Packet* packet)
{
   ConnList* ptr;
   
   u_int32_t index = packet->hashPacket();

   printf("the hash value is %u in findconnection\n",index);

   for(ptr = connection_hash[index]; ptr != NULL; ptr = ptr->next)
   {
//printf("$$$$$$$$$$$$$$$$$$$\n");
      if(packet->match(ptr->val->iniPacket))
      	{
      		//ptr->val->lastAccess = current_time++;
		
	  	return ptr->val;
      	}
   }
   
   
   Packet* invertedPacket = packet->newInverted();
   index = invertedPacket->hashPacket();

   printf("the hash value is %u in findconnection\n",index);

   for(ptr = connection_hash[index]; ptr != NULL; ptr = ptr->next)
   {
printf("$$$$$$$$$$$$$$$$$$$\n");
      if(invertedPacket->match(ptr->val->iniPacket))
      	{
		
      	   delete invertedPacket;
	   //ptr->val->lastAccess = current_time++;
	   return ptr->val;
      	}
   }
   delete invertedPacket;
   return NULL;
}*/

bool Connection::ReassemblePacket(Packet* packet)
{
   int src_index,i;
   u_int32_t src,dst;
   
   printf("ressemblePacket\n");

/*
   if(packet->Outgoing())
   	latDirection = dir_outgoing;
   else
   	latDirection = dir_incoming;*/

   src = (u_int32_t)packet->sip.s_addr;
   dst = (u_int32_t)packet->dip.s_addr;

   src_index = -1;

   for(i = 0; i < 2; i++)
   {
      if(src == ipAddr[i] && packet->sport == portAddr[i])
	//if(src == ipAddr[i])
	{
	  	src_index = i;
		break;
	}
   }
   
   
   switch(curState)
   {
      case stat_empty:
	  if(packet->flags & TH_SYN)
	  {
	     Process_syn_packet(packet);
		 return true;
	  }
	  break;
      case stat_syn:
	  //printf("@@@@@@@@@@@@@@@@@@@@@\n");
	  if((packet->flags & TH_SYN) && (packet->flags & TH_ACK))
	  {
	     Process_synack_packet(packet);
		 return true;
	  }
	  break;
      case stat_synack:
	  if(packet->flags & TH_SYN)
	  {
	     Process_syn_packet(packet);
		 return true;
	  }
	  else if(packet->flags & TH_ACK)
	  {
	     printf("find a ack,connection established\n");
	     curState = stat_ack;
		 return true;
	  }
	  break;
      case stat_ack:
	  if(packet->flags & TH_SYN)
	  {
	     Process_syn_packet(packet);
		 return true;
	  }
	  else if(packet->flags & TH_FIN)
	  {
	  	Process_fin_packet(packet);
		return true;
	  }
	  else if(packet->len > 0)
	  {
	  	Process_data_packet(packet,src_index);
		curState = stat_packack;
		return true;
	  }
	  break;
	case stat_packack:
		if(packet->flags & TH_FIN)
		{
			Process_fin_packet(packet);
			return true;
		}
		else if(packet->len && packet->flags & TH_ACK)
		{
			Process_data_packet(packet,src_index);
			return true;
		}
		else if(packet->flags & TH_ACK)
		{
			return true;
		}
		break;
	default:
		break;
			
   }

   return false;
}


void Connection::Process_syn_packet(Packet * packet)
{
   u_int32_t sequence;
   printf("find a syn\n");
   printf("src %u ,dst %u , srcPort %d ,dstPort %d\n",packet->sip,packet->dip,packet->sport,packet->dport);
  /* if(packet->Outgoing())
   	synDirection = dir_outgoing;
   else
   	synDirection = dir_incoming;*/

   if(portAddr[0] == 0 ||ipAddr[0] != packet->sip.s_addr || portAddr[0] != packet->sport)
   {
      ipAddr[0] = (u_int32_t)packet->sip.s_addr;
      portAddr[0] = packet->sport;
   }
   sequence = (u_int32_t)packet->seq;
   //seq[0] = sequence + packet->len;
   seq[0] = sequence;
   if(packet->flags & TH_SYN)
   	seq[0]++;
   printf("seq %u\n",sequence);

   curState = stat_syn;
}


void Connection::Process_synack_packet(Packet * packet)
{
   u_int32_t sequence;
   printf("find a syn,ack\n");

   if(portAddr[1] == 0 || ipAddr[1] != packet->dip.s_addr || portAddr[0] != packet->dport)
   {
      //ipAddr[1] = (u_int32_t)packet->dip.s_addr;
      //portAddr[1] = packet->dport;
      ipAddr[1] = (u_int32_t)packet->sip.s_addr;
      portAddr[1] = packet->sport;
   }
   sequence = (u_int32_t)packet->seq;
   //seq[1] = sequence + packet->len;
   seq[1] = sequence;
   if(packet->flags & TH_SYN)
   	seq[1]++;

   printf("seq %u\n",seq[1]);

   curState = stat_synack;
}

void Connection::Process_fin_packet(Packet * packet)
{
	int i;
	FILE *fp;
	printf("process fin\n");
        printf("currMessSize:%d\n",currMessCount);
	if(currMessCount < MESS_SIZE && currMessCount >= 0)
	{
	  //for(i = 0; i <= currMessCount; i++)
	  //	printf("%d's mess:%d,",i,messages[i]);
	  //printf("\n");

		fp = fopen("data","a");
		
		fprintf(fp,"%d,%d\n",2,currMessCount+1);
		for(i = 0; i <= currMessCount; i++)
		{
		        printf("%d's mess:%d,",i,messages[i]);
			if(i!=currMessCount)
				fprintf(fp,"%d,",abs(messages[i]));
			else
				fprintf(fp,"%d\n",abs(messages[i]));
		}
		for(i = 0; i <= currMessCount; i++)
		{
			if(i!=currMessCount)
				fprintf(fp,"%d,",(messages[i]>=0?0:1));
			else
				fprintf(fp,"%d\n",(messages[i]>=0?0:1));
		}
		fclose(fp);	
		printf("\n");

		Message mess;
		memset(&mess,0,sizeof(Message));
		mess.inUsed = 1;
		mess.addr.src = ipAddr[0];
		mess.addr.dst = ipAddr[1];
		mess.addr.sPort = portAddr[0];
		mess.addr.dPort = portAddr[1];
		for(i = 0;i <= currMessCount;i++)
			mess.messages[i] = messages[i];
		InsertIntoQueue(mess,currMessCount + 1);
		//InsertIntoDB(messages,currMessCount);
	}
	SET_BIT(flags,CONN_FINISHED);
	//closeFile();
	curState = stat_fin;
	
	//delete this;
}


void Connection::Process_data_packet(Packet * packet, int index)
{
   u_int32_t newseq;
   u_int32_t sequence = (u_int32_t)packet->seq;
   u_int32_t length = packet->len;
   Packet* tmp_frag;
   printf("process data\n");
   printf("seq : %u , save seq :%u\n",sequence,seq[0]);
   if(index < 0)
      return;
   if(sequence < seq[index])
   {
      newseq = sequence + length;
	  if(newseq > seq[index])
	  {
	  	//u_int32_t new_len = seq[index] - sequence;
		//length -= new_len;
		sequence = seq[index];
		length = newseq - sequence;
	  }
   }

   if(sequence == seq[index])
   {
   	seq[index] += length;
	if(packet->flags & TH_SYN)
		seq[index]++;
	if(length > 0)
	{
	   char output[200];
	   int len,retLen;
	   
	   /*if(packet->curDirection == synDirection)
	   	len = length;
	   else
	   	len = -length;*/
	   if(packet->match(iniPacket))
		len = length;
	   else
		len = -length;

	   retLen = sprintf(output,"%3d.%3d.%3d.%3d %5d %3d.%3d.%3d.%3d %5d %d %u %u %d\n",
	   	(u_int8_t)((packet->sip.s_addr & 0xff000000) >> 24),
	   	(u_int8_t)((packet->sip.s_addr & 0x00ff0000) >> 16),
	   	(u_int8_t)((packet->sip.s_addr & 0x0000ff00) >> 8),
	   	(u_int8_t)((packet->sip.s_addr & 0x000000ff)),
	   	packet->sport,
	   	(u_int8_t)((packet->dip.s_addr & 0xff000000) >> 24),
	   	(u_int8_t)((packet->dip.s_addr & 0x00ff0000) >> 16),
	   	(u_int8_t)((packet->dip.s_addr & 0x0000ff00) >> 8),
	   	(u_int8_t)((packet->dip.s_addr & 0x000000ff)),
	   	packet->dport,
	   	len,
	   	packet->seq,
	   	packet->ack,
	   	packet->flags);

	   	//write_packet_data(output,retLen);
		
		if(currMessCount < MESS_SIZE)
			construct_App_Mess(packet);
		
	}
	while(checkPacket(index))
		;
   }
   else
   {
   	if(length > 0 && sequence > seq[index])
   	{
   		//tmp_frag = new PackList();
		//tmp_frag->
		frags[index]->add(packet);
   	}
   }
   
   
}

void Connection::construct_App_Mess(Packet* packet)
{
	int len,i;
	if(packet->match(iniPacket))
		len = packet->len;
	   else
		len = -packet->len;
	//if(latDirection == packet->curDirection)
	printf("construct Message\n");
	if(packet->match(prePacket))
	{
		printf("match !!! ");
		/*if(prePacket == NULL && packet->flags & TH_PUSH)
		{
			messages[currMessCount++] = len;
			prePacket = new Packet(*packet);
		}
		else if(prePacket == NULL)
		{
			messages[currMessCount] = len;
			prePacket = new Packet(*packet);
		}*/

		/*
		if(packet->ack == prePacket->ack && packet->flags & TH_PUSH)
		{
			messages[currMessCount++] += len;
			delete prePacket;
			prePacket = new Packet(*packet);
		}
		else if(packet->ack == prePacket->ack)
		{
			messages[currMessCount] += len;
			delete prePacket;
			prePacket = new Packet(*packet);
		}
		else
		{
			messages[++currMessCount] = len;
			delete prePacket;
			prePacket = new Packet(*packet);
		}*/
		/*
		messages[currMessCount] += len;
		

		if(packet->flags & TH_PUSH && packet->ack != prePacket->ack)
		{
			printf("Mess++\n");
			currMessCount++;
		}
		*/

		if(packet->ack != prePacket->ack)
		{
			//messages[++currMessCount] = len;
			currMessCount++;
			if(currMessCount < MESS_SIZE)
			   messages[currMessCount] = len;
		}
		else
			messages[currMessCount] += len;
		delete prePacket;
		prePacket = new Packet(*packet);
	}
	else
	{
		printf("not match !!!");
		currMessCount++;
		
		if(currMessCount < MESS_SIZE)
			messages[currMessCount] = len;
		/*
		messages[currMessCount] = len;
		
		if(packet->flags & TH_PUSH && packet->ack != prePacket->ack)
		{
			printf("Mess++\n");
			currMessCount++;
		}*/
		delete prePacket;
		prePacket = new Packet(*packet);
	}

	if(currMessCount == MESS_SIZE)
	{
		//insert data into database
	  //	printf("insert data into database\n");
	  //printf("message is\n");
	  //for(i = 0; i < MESS_SIZE; i++)
	  //	printf("%d's mess:%d,",i,messages[i]);
	  //printf("\n");
	  	FILE *fp;

		fp = fopen("data","a");
		
		fprintf(fp,"%d,%d\n",2,MESS_SIZE);
		for(i = 0; i < MESS_SIZE; i++)
		{
		        printf("%d's mess:%d,",i,messages[i]);
			if(i!=MESS_SIZE - 1)
				fprintf(fp,"%d,",abs(messages[i]));
			else
				fprintf(fp,"%d\n",abs(messages[i]));
		}
		for(i = 0; i < MESS_SIZE; i++)
		{
			if(i!=MESS_SIZE - 1)
				fprintf(fp,"%d,",(messages[i]>=0?0:1));
			else
				fprintf(fp,"%d\n",(messages[i]>=0?0:1));
		}
		fclose(fp);	
		printf("\n");

		Message mess;
		memset(&mess,0,sizeof(Message));
		mess.inUsed = 1;
		mess.addr.src = ipAddr[0];
		mess.addr.dst = ipAddr[1];
		mess.addr.sPort = portAddr[0];
		mess.addr.dPort = portAddr[1];
		for(i = 0;i < currMessCount;i++)
			mess.messages[i] = messages[i];
		InsertIntoQueue(mess,currMessCount);
		//InsertIntoDB(messages,currMessCount);
		
		//when get 5 messages.it is enough,the currMessCount start from 0
		SET_BIT(flags,CONN_FINISHED);
		
	}
}

bool Connection::checkPacket(int index)
{
	PackListNode* prev = NULL;
	PackListNode* current;
	current = frags[index]->content;
	while(current != NULL)
	{
		if(current->val->seq == seq[index])
		{
			if(current->val->len > 0 )
			{
				Packet* packet = current->val;
				char output[200];
	   			int len,retLen;
	   
	   			if(packet->match(iniPacket))
				   len = packet->len;
	   			else
				   len = -packet->len;

	   			retLen = sprintf(output,"%3d.%3d.%3d.%3d %5d %3d.%3d.%3d.%3d %5d %d %u %u %d\n",
	   				(u_int8_t)((packet->sip.s_addr & 0xff000000) >> 24),
	   				(u_int8_t)((packet->sip.s_addr & 0x00ff0000) >> 16),
	   				(u_int8_t)((packet->sip.s_addr & 0x0000ff00) >> 8),
	   				(u_int8_t)((packet->sip.s_addr & 0x000000ff)),
	   				packet->sport,
	   				(u_int8_t)((packet->dip.s_addr & 0xff000000) >> 24),
	   				(u_int8_t)((packet->dip.s_addr & 0x00ff0000) >> 16),
	   				(u_int8_t)((packet->dip.s_addr & 0x0000ff00) >> 8),
	   				(u_int8_t)((packet->dip.s_addr & 0x000000ff)),
	   				packet->dport,
	   				len,
	   				packet->seq,
	   				packet->ack,
	   				packet->flags);

				//write_packet_data(output,retLen);
				if(currMessCount < MESS_SIZE)
					construct_App_Mess(packet);
				
			}
			seq[index] += current->val->len;
			if(prev != NULL)
				prev->next = current->next;
			else
				frags[index]->content = current->next;

			current->next= NULL;
			delete current;
			current = NULL;
			return true;
		}
		
		prev = current;
		current = current->next;
	}
	return false;
}

/*
void Connection::write_packet_data(char * data, int length)
{
	if(fp == NULL)
		openFile();

	fwrite(data,length,1,fp);
	fflush(fp);

}*/

void InsertIntoQueue(Message mess,int size)
{
	EnQueue(mess);
	SignalQueueHasElement();
}

void InsertIntoDB(int mess[],int size)
{

   MYSQL *conn;
   MYSQL_RES *res;
   MYSQL_ROW row;

   char *server = "localhost";
   char *user = "root";
   char *password = "198783"; 
   char *database = "limeDB";

   conn = mysql_init(NULL);

 
   if (!mysql_real_connect(conn, server,
         user, password, database, 0, NULL, 0)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   }

/*
   if (mysql_query(conn, "show tables")) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   }*/
   char insertSql[200];
   sprintf(insertSql,"insert into Message set Mess1=%d,Mess2=%d,Mess3=%d,Mess4=%d,Mess5=%d",
	mess[0],mess[1],mess[2],mess[3],mess[4]);

	mysql_query(conn,insertSql);

   //res = mysql_use_result(conn);


   /*printf("MySQL Tables in mysql database:\n");
   while ((row = mysql_fetch_row(res)) != NULL)
      printf("%s \n", row[0]);*/

 
   //mysql_free_result(res);
   mysql_close(conn);

   //send the Message to HCRF process using IPC method

}
