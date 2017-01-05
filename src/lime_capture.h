#ifndef __LIME_CAPTURE_H
#define __LIME_CAPTURE_H

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <iostream>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
# include <sys/time.h>
# include <time.h>

/**************************** Constants ***********************************/
#define MAX_FD_GUESS        64
#define NUM_RESERVED_FDS    5     /* number of FDs to set aside */
#define HASH_SIZE           1009  /* prime number near 1000 */
#define MTU_LEN             65535 /* largest possible MTU we'll see */

#define HASHKEYSIZE 44

#define PROGNAME_WIDTH 27

#define CONN_FINISHED		(1 << 0)
#define CONN_FILE_EXISTS	(1 << 1)


#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20
#define	TH_ECE	0x40
#define	TH_CWR	0x80


/**************************** Structures **********************************/

class Local_addr
{
private:
	in_addr_t addr;
	int16_t sa_family;

public:
	Local_addr* next;
	Local_addr(in_addr_t _addr,Local_addr* _next = NULL)
	{
	   addr = _addr;
	   next = _next;
	   sa_family = AF_INET;
	}

	bool contains(const in_addr_t& _addr);	
};


/***************************** Macros *************************************/
#define MALLOC(type,num) (type*)check_malloc((num) * sizeof(type))

#define IS_SET(vector,flag) ((vector) & (flag))
#define SET_BIT(vector,flag) ((vector) |= (flag))

#endif