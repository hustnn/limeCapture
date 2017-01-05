#ifndef __CONNECTION_H
#define __CONNECTION_H

#include "Packet.h"
//#include "connection_hashMap.h"
//#include "cursor_mem_pool.h"

#include "shared_queue.h"

using namespace std;



#define MESS_SIZE 5

//#define CONN_FINISHED		(1 << 0)
//#define CONN_FILE_EXISTS	(1 << 1)

enum tcpState
{
   stat_empty,
   stat_syn,
   stat_synack,
   stat_ack,
   stat_packack,
   stat_fin,
};

class hashMapElement;

class Connection
{
public:
   int32_t ipAddr[2];
   u_int16_t portAddr[2];
   u_int32_t seq[2];
   u_int32_t ack[2];

   u_int32_t iseq;
   int flags;
   tcpState curState;

   int16_t currMessCount;
   int messages[MESS_SIZE];

   timeval connTimestamp;
   
   Packet* iniPacket;
   Packet* prePacket;
   PackList* frags[2];

   hashMapElement* owner;

   void* operator new(size_t size);
   void operator delete(void* p);

   Connection();
   Connection(Packet* packet);

	~Connection();

	void add(Packet* packet);

	void resetTcpReassembly();

	//FILE* openFile();

	//FILE* attemptFopen(char* filename);

	//int closeFile();

	bool ReassemblePacket(Packet* packet);

	bool checkPacket(int index);

	void Process_syn_packet(Packet* packet);

	void Process_synack_packet(Packet* packet);

	void Process_data_packet(Packet* packet,int index);

	void Process_fin_packet(Packet* packet);

	void write_packet_data(char* data,int length);

	void construct_App_Mess(Packet* packet);
};

/*
class Connection
{
public:
	Packet* iniPacket;
	Packet* refpacket;

	u_int32_t sumSent;
	u_int32_t sumRecv;
	FILE* fp;
	int flags;
	u_int32_t iseq;

	u_int16_t currMessCount;
	int messages[MESS_SIZE];
	Packet* prePacket;

	u_int32_t hashValue;
	tcpState curState;
	int lastAccess;

	direction synDirection;
	direction latDirection;
	timeval connTimestamp;
	
	Connection(Packet* packet);

	~Connection();

	void add(Packet* packet);

	void resetTcpReassembly();

	FILE* openFile();

	FILE* attemptFopen(char* filename);

	int closeFile();

	bool ReassemblePacket(Packet* packet);

	bool checkPacket(int index);

	void Process_syn_packet(Packet* packet);

	void Process_synack_packet(Packet* packet);

	void Process_data_packet(Packet* packet,int index);

	void Process_fin_packet(Packet* packet);

	void write_packet_data(char* data,int length);

	void construct_App_Mess(Packet* packet);

private:
	u_int32_t ipAddr[2];
	u_int16_t portAddr[2];
	u_int32_t seq[2];
	u_int32_t ack[2];
	//PackList* sent_packets;
	//PackList* recv_packets;
	PackList* frags[2];
	
};*/




class ConnList
{
public:
	ConnList (Connection * m_val = NULL, ConnList * m_next = NULL)
	{
	    val = m_val; next = m_next;
	}
	Connection * val;
	ConnList * next;
};

Connection* findConnection(Packet* packet);

int conn_compare(const void* a,const void* b);

void sort_fds();


void initCoonState();

void contract_fd_ring();

void InsertIntoDB(int mess[],int size);

void InsertIntoQueue(Message mess,int size);

#endif