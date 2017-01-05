#ifndef __CURSOR_MEM_POOL_H
#define __CURSOR_MEM_POOL_H

#define MESS_SIZE 5
#define POOL_SPACE_SIZE 100000

#include "connection.h"
#include "lime_capture.h"

//class Connection;

/*
typedef struct _hashMapElement
{
   u_int32_t no;
   u_int32_t memPoolNext;

   int32_t key;
   struct _hashMapElement* hashMapNext;
   Connection* test;
   //class Connection data;
}hashMapElement;*/

class hashMapElement
{
public:
   u_int32_t no;
   u_int32_t memPoolNext;

   int32_t key;
   hashMapElement* hashMapNext;
   Connection data;
};

/*
typedef struct Addr
{
   int32_t src;
   int32_t dst;
   u_int16_t srcPort;
   u_int16_t dstPort;
}Addr_t;

typedef struct Node
{
   u_int32_t no;
   Addr_t addr;
   int16_t messages[MESS_SIZE];
   u_int8_t curSize;
   u_int32_t next;

   //for hashmap
   u_int32_t hashKey;
   Node* nextElement;
}Node_t;*/

typedef int ptrToNode;

typedef ptrToNode Position;

static Position freeListHeader = 0;
static Position freeListTail = 0;
static Position usedListHeader = -1;
static Position usedListTail = -1;


extern hashMapElement* CursorAlloc();
extern void CursorFree(hashMapElement* ptr);

extern void InitializeCursorSpace();

bool MakeEmpty(Position listHeader);

bool IsEmpty(Position listHeader);
bool IsLast(Position p);
bool IsFirst(Position p,Position ListHeader);

//Position Find(Addr_t addr,Position list);
static Position FindPrevious(Position p,Position listHeader);
static void Delete(Position p,Position* listHeader,Position* listTail);//delete,for freelist delete header,for usedlist delete from any position 
static void Insert(Position p,Position*listHeader,Position* listTail); //insert into tail both freelist or uesedlist
//void DeleteList(Position list);
//Position First(Position list);
//Position Advance(Position p);
//hashMapElement* Retrive(Position p,Position list);
//void Reset(Position p);
static Position FindPrevious(Position p,Position listHeader);

Position FindOuttimeBlock(Position listHeader);
//bool IsSameAddr(Addr_t one,Addr_t other);

#endif