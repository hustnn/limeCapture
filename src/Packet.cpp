#include "Packet.h"
//#include "lime_capture.h"


#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <assert.h>

#include <netdb.h>
#include <ifaddrs.h>
#include <math.h>


#define ADDR_SIZE 12

Local_addr* local_addrs = NULL;

void getLocal(const char* device)
{
   int sock;
   struct ifreq ifr;
   struct sockaddr_in* saddr;

   struct ifaddrs *ifaddr, *ifa;
   int family, s;
   char host[NI_MAXHOST];

   if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
   }

   for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
                s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                               host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != 0) {
                        printf("getnameinfo() failed: %s\n", gai_strerror(s));
                        exit(EXIT_FAILURE);
                }
                printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
		saddr = (struct sockaddr_in*)ifaddr;
   		local_addrs = new Local_addr(saddr->sin_addr.s_addr,local_addrs);
        }
   }

   
   

#if 0
   if(sock = socket(AF_INET,SOCK_RAW,htons(0x0806)) < 0)
   //if(sock = socket(AF_INET,SOCK_DGRAM, 0) < 0)
   {
      die("create socket failed");
   }

   //ifr.ifr_addr.sa_family = AF_INET;
   //strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
   strcpy(ifr.ifr_name,device);
   
   if(ioctl(sock,SIOCGIFADDR,&ifr) < 0)
   {
      die("ioctl get ip address failed!");
   }

   saddr = (struct sockaddr_in*)&ifr.ifr_addr;
   local_addrs = new Local_addr(saddr->sin_addr.s_addr,local_addrs);
#endif

   
}

void PackList::add(Packet* p)
{
   if(content == NULL)
   {
      	content = new PackListNode(new Packet(*p));
	return;
   }

   content = new PackListNode(new Packet(*p),content);
}

Packet::Packet(in_addr _sip,in_addr _dip,u_int16_t _sport,u_int16_t _dport,u_int32_t _len,timeval _time,
	u_int32_t _seq,u_int32_t _ack,u_char tcpFlags,direction _dir)
{
   sip = _sip;
   dip = _dip;
   sport = _sport;
   dport = _dport;
   len = _len;
   time = _time;
   seq = _seq;
   ack = _ack;
   flags = tcpFlags;
   dir = _dir;
   hashstring = NULL;
}

Packet::Packet(const Packet& old_packet)
{
   sip = old_packet.sip;
   dip = old_packet.dip;
   sport = old_packet.sport;
   dport = old_packet.dport;
   len = old_packet.len;
   time = old_packet.time;
   seq = old_packet.seq;
   ack = old_packet.ack;
   flags = old_packet.flags;
   dir = old_packet.dir;
   if(old_packet.hashstring == NULL)
   	hashstring = NULL;
   else
   	hashstring = strdup(old_packet.hashstring);
}

Packet* Packet::newInverted()
{
   return new Packet(dip,sip,dport,sport,len,time,seq,ack,flags,dir);
}

bool Packet::Outgoing()
{
   assert(local_addrs != NULL);

   switch(dir)
   {
      case dir_incoming:
	  return false;
      case dir_outgoing:
	  return true;
      case dir_unknown:
	  bool islocal;
	  islocal = local_addrs->contains(sip.s_addr);
	  if(islocal)
	  {
	      dir = dir_outgoing;
	      return true;
	  }
	  else
	  {
	      dir = dir_incoming;
	      return false;
	  }
   }

   return false;
}

bool sameinaddr(in_addr one,in_addr two)
{
   return one.s_addr == two.s_addr;
}

bool Packet::match(Packet* other)
{
   if(other == NULL)
	return false;
   return (sport == other->sport) && (dport == other->dport) && sameinaddr(sip,other->sip) && sameinaddr(dip,other->dip);
}

char* Packet::gethashstring()
{
   if(hashstring != NULL)
   	return hashstring;

   hashstring = (char*)malloc(HASHKEYSIZE * sizeof(char));

   char* local_string = (char*)malloc(16 * sizeof(char));
   char* remote_string = (char*)malloc(16 * sizeof(char));

   inet_ntop(AF_INET,&sip,local_string,15);
   inet_ntop(AF_INET,&dip,remote_string,15);

   if(Outgoing())
   {
      snprintf(hashstring,HASHKEYSIZE * sizeof(char),"%s:%d-%s:%d",local_string,sport,remote_string,dport);
   }
   else
   {
      snprintf(hashstring,HASHKEYSIZE * sizeof(char),"%s:%d-%s:%d",remote_string,dport,local_string,sport);
   }

   free(local_string);
   free(remote_string);

   return hashstring;
}


/*
u_int32_t Packet::hashPacket()
{
   u_int32_t val = (sport & 0xff) | ((dport & 0xff) << 8) | ((sip.s_addr & 0xff) << 16) | ((dip.s_addr & 0xff) << 24);
   return val % HASH_SIZE;
}*/

int32_t Packet::hashPacket()
{
   int32_t _src = sip.s_addr;
   int32_t _dst = dip.s_addr;
   u_int16_t _sport = sport;
   u_int16_t _dport = dport;
   int i;
   int32_t val = 0;
   double base = 31;
   double k;
   char tmp[ADDR_SIZE];
   char *s_src,*s_dst,*s_srcPort,*s_dstPort;
   s_src = (char*)(&_src);
   s_dst = (char*)(&_dst);
   s_srcPort = (char*)(&sport);
   s_dstPort = (char*)(&dport);
   memcpy(tmp,s_src,4);
   memcpy(tmp + 4,s_dst,4);
   memcpy(tmp + 8,s_srcPort,2);
   memcpy(tmp+10,s_dstPort,2);
   for(i = 0;i < ADDR_SIZE;i++)
   {
      k = (double)(11 - i);
      val += tmp[i] * pow(base,k);
   }
   return val;
}

#define RING_SIZE 6

char* Packet::packetFileName()
{
   static char ring_buffer[RING_SIZE][70];
   static int ring_pos = 0;

   ring_pos = (ring_pos + 1) % RING_SIZE;

   sprintf(ring_buffer[ring_pos],
   	"%3d.%3d.%3d.%3d:%5d-%3d.%3d.%3d.%3d:%5d-%u:%u",
   	(u_int8_t)((sip.s_addr & 0xff000000) >> 24),
   	(u_int8_t)((sip.s_addr & 0x00ff0000) >> 16),
   	(u_int8_t)((sip.s_addr & 0x0000ff00) >> 8),
   	(u_int8_t)((sip.s_addr & 0x000000ff)),
   	sport,
   	(u_int8_t)((dip.s_addr & 0xff000000) >> 24),
   	(u_int8_t)((dip.s_addr & 0x00ff0000) >> 16),
   	(u_int8_t)((dip.s_addr & 0x0000ff00) >> 8),
   	(u_int8_t)((dip.s_addr & 0x000000ff)),
   	dport,
   	time.tv_sec,
   	time.tv_usec
   	);

   return ring_buffer[ring_pos];
}

