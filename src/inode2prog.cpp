#include "inode2prog.cpp"
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <map>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

std::map<unsigned long,struct prg_node*> inodeproc;

bool is_number(char* str)
{
   while(*str)
   {
      if(!isdigit(*str))
	  return false;
      str++;
   }
   return true;
}

unsigned long str2ulong(char* str)
{
   unsigned long retval = 0;
   while((*str > '0') && (*str < '9'))
   {
      retval *= 10;
      retval += *str - '0';
      str++;
   }
   return retval;
}


int str2int(char* str)
{
   int retval = 0;
   while((*str > '0') && (*str < '9'))
   {
      retval *= 10;
      retval += *str - '0';
      str++;
   }
   return retval;
}

char* getprogname(char* pid)
{
   int filenamelen = 14 + strlen(pid) + 1;
   int bufsize = 80;
   char buffer [bufsize];
   char * filename = (char *) malloc (filenamelen);
   snprintf (filename, filenamelen, "/proc/%s/cmdline", pid);
   int fd = open(filename, O_RDONLY);
   if(pid < 0)
   {
      debug_real("error opening %s : %s",filename,strerror(errno));
      return NULL;
   }
   int length = read (fd, buffer, bufsize);
   if(close(fd))
   {
      die("error closing file %s",filename);
   }

   free(filename);

   if(length < bufsize - 1)
   	buffer[length] = '\0';

   char* retval = buffer;

   return strdup(retval);
}

void setnode(unsigned long inode,struct prg_node* newnode)
{
   if(inodeproc[inode] != NULL)
   	free(inodeproc[inode])
   inodeproc[inode] = newnode;
}

void get_info_by_linkname(char* pid,char* linkname)
{
   if(strncmp(linkname,"socket:[",8) == 0)
   {
      char * ptr = linkname + 8;
      unsigned long inode = str2ulong(ptr);
      char * progname = getprogname (pid);
      struct prg_node* newnode = (struct prg_node*)malloc(sizeof(struct prg_node));
      newnode->inode = inode;
      newnode->pid = str2int(pid);
      strncpy(newnode->name,progname,PROGNAME_WIDTH);
      free(progname);
      setnode(inode,newnode);
   }
}

/* updates the `inodeproc' inode-to-prg_node 
 * for all inodes belonging to this PID 
 * (/proc/pid/fd/42)
 */
void get_info_for_pid(char* pid)
{
   size_t dirlen = 10 + strlen(pid);
   char* dirname = (char*)malloc(dirlen * sizeof(char));
   snprintf(dirname,dirlen,"/proc/%s/fd",pid);

   DIR* dir = opendir(dirname);
   if(!dir)
   {
      debug_real("can't open dir %s",dirname);
      free(dirname);
      return;
   }

   dirent* entry;
   while((entry = readdir(dir)))
   {
      if(entry->d_type != DT_LNK)
	  	continue;
      int fromlen = dirlen + strlen(entry->d_name) + 1;
      char * fromname = (char *) malloc (fromlen * sizeof(char));
      snprintf (fromname, fromlen, "%s/%s", dirname, entry->d_name);

      int linklen = 80;
      char linkname [linklen];
      int usedlen = readlink(fromname, linkname, linklen-1);
      if(usedlen == -1)
      	{
      	   free(fromname);
	   continue;
      	}

      assert(usedlen < linklen);
      linkname[usedlen] = '\0';
      get_info_by_linkname(pid,linkname);
      free(linkname);

   }
   closedir(dir);
   free(dirname);
}

/* updates the `inodeproc' inode-to-prg_node mapping 
 * for all processes in /proc
 */
void reread_mapping()
{
   DIR* proc = opendir("/proc");

   if(proc == 0)
   	die("error reading dir");

   dirent* entry;

   while((entry = readdir(proc)))
   {
      if(entry->d_type != DT_DIR)
	  continue;
      if(!is_number(entry->d_name))
	  continue;
      get_info_for_pid(entry->d_name);
   }

   closedir(proc);
}


struct prg_node * findPID (unsigned long inode)
{
   struct prg_node* node = inodeproc[inode];

   if(node != NULL)
   	return node;

   reread_mapping();

   node = inodeproc[inode];
   return node;
}
