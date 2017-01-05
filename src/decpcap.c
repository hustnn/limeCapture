#include "decpcap.h"
#include "util.h"
#include "Packet.h"
#include "connection.h"

#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>


void dl_ethernet(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
   u_int caplen = h->caplen;
   u_int length = h->len;
   struct ether_header* eth_header = (struct ether_header*)p;


   if(caplen < length)
   	debug_real("only capture %d bytes of %d bytes frame",caplen,length);

   if(caplen < sizeof(struct ether_header))
   {
      debug_real("received uncompleted ethernet frame");
      return;
   }

   switch(eth_header->ether_type)
   {
      	//case ETHERTYPE_IP:
	case 0x0008:
	  	process_ip(p + sizeof(struct ether_header),caplen - sizeof(struct ether_header),h);
		//printf("process ip!!!!!!!!!!!\n");
	  	break;
	//case ETHERTYPE_IPV6:
	case 0xDD86:
		break;
	default:
		//debug_real("unknown ethernet type");
		break;
   }
}

pcap_handler find_handler(int datalink_type, char *device)
{
   int i;
   struct
   {
      pcap_handler handler;
      int type;
   } handlers[] = 
   {
   	{dl_ethernet,DLT_EN10MB},
   	{NULL,0},
   };

   for(i = 0; handlers[i].handler != NULL; i++)
   {
   	if(handlers[i].type == datalink_type)
	   return handlers[i].handler;
   }

   fprintf(stdout,"unknown datalink type %d on device %s",datalink_type,device);
   return NULL;
}

void process_ip(const u_char *data, u_int32_t caplen,const struct pcap_pkthdr *h)
{
   const struct ip* ip_header = (struct ip*)data;
   u_int ip_header_len;
   u_int ip_total_len;

   if(caplen < sizeof(struct ip))
   {
      debug_real("received uncompleted IP datagram!");
      return;
   }

   if(ip_header->ip_p != IPPROTO_TCP)
   {
      debug_real("got non-TCP frame %d",ip_header->ip_p);
      return;
   }

   ip_total_len = ntohs(ip_header->ip_len);
   if(caplen < ip_total_len)
   	debug_real("capture only %ld bytes of %ld bytes IP datagram",(long)caplen,(long)ip_total_len);

   if(ntohs(ip_header->ip_off) & 0x1fff)
   {
      debug_real("throwing away fragment IP datagram");
      return;
   }

   ip_header_len = ip_header->ip_hl * 4;
   if(ip_header_len > ip_total_len)
   	return;

   process_tcp(data + ip_header_len,ip_total_len - ip_header_len,ip_header->ip_src,ip_header->ip_dst,h);
}

void process_tcp(const u_char *data, u_int32_t length, in_addr src,in_addr dst,const struct pcap_pkthdr *h)
{
   u_char flags = 0;
   struct tcphdr* tcp = (struct tcphdr*)data;
   u_int tcp_header_len = tcp->doff * 4;
   //printf("tcp len is %d\n",tcp_header_len);
   
   
   //flags = tcp->fin | tcp->syn | tcp->rst | tcp->psh | tcp->ack | tcp->urg;
   if(tcp->fin)
	flags |= TH_FIN;
   if(tcp->syn)
	flags |= TH_SYN;
   if(tcp->rst)
	flags |= TH_RST;
   if(tcp->psh)
	flags |= TH_PUSH;
   if(tcp->ack)
	flags |= TH_ACK;
   if(tcp->urg)
	flags |= TH_URG;

   printf("tcp flags is %d\n",flags);
   printf("seq %d\n",ntohl(tcp->seq));

   Packet* packet = new Packet(src,dst,ntohs(tcp->source),ntohs(tcp->dest),(length - tcp_header_len),h->ts,ntohl(tcp->seq),ntohl(tcp->ack_seq),flags);

   Connection* connection = findConnection(packet);

   if(connection != NULL)
	printf("#####################\n");
   
   if(connection == NULL && packet->flags & TH_SYN)
   {
      
      connection = new Connection(packet);
   }

   if(connection == NULL)
   {
	return;
   }
	
   //printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n");   

   bool ret = connection->ReassemblePacket(packet);
   delete packet;
	
   if(IS_SET(connection->flags,CONN_FINISHED))
   {
      printf("delete connection\n");
      delete connection;
   }
}
