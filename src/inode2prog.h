#ifndef __INODE2PROG_H
#define __INODE2PROG_H

#include "lime_capture.h"

struct prg_node
{
   unsigned long inode;
   int pid;
   char name[PROGNAME_WIDTH];
};

bool is_number(char* str);

unsigned long str2ulong(char* str);

int str2int(char* str);

char* getprogname(char* pid);

void setnode(unsigned long inode,struct prg_node* newnode);

void get_info_by_linkname(char* pid,char* linkname);

void get_info_for_pid(char* pid);

// reread the inode-to-prg_node-mapping
void reread_mapping ();

struct prg_node* findPID (unsigned long inode);

//void prg_cache_clear();
 

#endif