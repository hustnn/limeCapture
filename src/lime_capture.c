/***************************************************************************
 *   Copyright (C) 2010 by Administrator,,,   *
 *   hustnn@ubuntu   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "lime_capture.h"
#include "util.h"
#include "decpcap.h"
#include "connection.h"
#include "Packet.h"
#include "cursor_mem_pool.h"
#include "connection_hashMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <getopt.h>

int no_promisc = 0;

char error[PCAP_ERRBUF_SIZE];


bool Local_addr::contains(const in_addr_t& _addr)
{
   if((sa_family == AF_INET) && (addr == _addr))
   {
      return true;
   }
   if(next == NULL)
      return false;
   return next->contains(_addr);
}

int main(int argc, char *argv[])
{
   printf("Hello, world!\n");
   extern char *optarg;
   int arg,dlt;
   pcap_t *pd;
   pcap_handler handler;

   char* device = "eth0";

   //initCoonState();
   InitializeCursorSpace();
   HashMapInit();
   CreateSharedQueue();

   while((arg = getopt(argc,argv,"i:p")) != EOF)
   {
   	switch(arg)
   	{
   	   case 'i':
		device = optarg;
		break;
	   case 'p':
		no_promisc = 1;
		break;
	   default:
		break;
   	}
   }

   no_promisc = 1;	
   if(device == NULL)
   	if((device = pcap_lookupdev(error)) == NULL)
		die("%s",error);

printf("Hello, world!\n");


   if((pd = pcap_open_live(device,MTU_LEN,!no_promisc,1000,error)) == NULL)
	printf("%s",error);
   	//die("%s",error);

   setuid(getuid());

   dlt = pcap_datalink(pd);
   handler = find_handler(dlt, device);

   getLocal(device);

   if(pcap_loop(pd,-1,handler,NULL) < 0)
   	die("%s",pcap_geterr(pd));

   HashMapFree();
   CloseSharedQueue();

   return 0;
}
