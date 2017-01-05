#ifndef __PACKET_H
#define __PACKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lime_capture.h"
#include "util.h"

enum direction
{
   dir_unknown,
   dir_incoming,
   dir_outgoing
};

class Packet
{
public:
	in_addr sip;
	in_addr dip;
	u_int16_t sport;
	u_int16_t dport;
	u_int32_t len;
	timeval time;
	u_int32_t seq;
	u_int32_t ack;
	u_char flags;

	direction curDirection;

	Packet(in_addr _sip,in_addr _dip,u_int16_t _sport,u_int16_t _dport,u_int32_t _len,timeval _time,u_int32_t _seq,u_int32_t _ack,u_char tcpFlags,direction _dir = dir_unknown);

	Packet(const Packet& old);
	~Packet()
	{
	   if(hashstring != NULL)
	   {
	      free(hashstring);
	      hashstring = NULL;
	   }
	}

	Packet* newInverted();

	bool Outgoing();

	bool match(Packet* other);

	char* gethashstring();

	//u_int32_t hashPacket();
	int32_t hashPacket();

	char* packetFileName();
	
private:
	direction dir;
	char* hashstring;
	
};

class PackListNode
{
public:
	Packet* val;
	PackListNode* next;

	PackListNode(Packet* _val,PackListNode* _next = NULL)
	{
	   val = _val;
	   next = _next;
	}

	~PackListNode()
	{
	   delete val;
	   if(next !=NULL)
	   	delete next;
	}
};

class PackList
{
public:
	PackListNode* content;

public:
	PackList()
	{
	   content = NULL;
	}

	PackList(Packet* val)
	{
	   content = new PackListNode(val);
	}

	~PackList()
	{
	   if(content != NULL)
	   	delete content;
	}

	void add(Packet* p);
};

void getLocal(const char* device);

#endif