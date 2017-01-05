#include <stdio.h>
#include <iostream>
#include <strings.h>
#include <string>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pwd.h>
#include <map>

#include "process.h"
#include "lime_capture.h"
#include "conninode.h"
#include "inode2prog.h"


extern Local_addr* local_addrs;

/*connection-inode table,from the /proc/net/tcp
 *key format:1.2.3.4:5-1.2.3.4:5
 */
extern std::map <std::string, unsigned long> conninode;

Process * unknowntcp;
ProcList * processes;

void process_init()
{
   unknowntcp = new Process (0, "", "unknown TCP");
   processes = new ProcList (unknowntcp, NULL);
}

Process* findProcess(struct prg_node* node)
{
   ProcList* current = processes;
   while(current != NULL)
   {
      Process* currentProc = current->getVal();
      assert(currentProc != NULL);
      if(node->pid == currentProc->pid)
	  	return currentProc;
      current = current->next;
   }
   return NULL;
}