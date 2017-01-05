#include "cursor_mem_pool.h"

static hashMapElement CURSOR_SPACE[POOL_SPACE_SIZE];

/**************************************
 ******malloc free node to use*********
**************************************/
hashMapElement* CursorAlloc()
{
   Position p;
   hashMapElement* ret;
   if(IsEmpty(freeListHeader))
   {
      p = -1; //now just return -1, in fact it should choose a timeout block to free and used in order to non block recv data
      return NULL;
   }
   else
   {
      /*
      if(IsEmpty(freeListHeader))
         return -1;
      p = freeListHeader;
      freeListHeader = CURSOR_SPACE[p].next; //alloc free space ,delete from freelist header
      */
      p = freeListHeader;
      Delete(p,&freeListHeader,&freeListTail);
      //Reset(p);
      Insert(p,&usedListHeader,&usedListTail);
      ret = &(CURSOR_SPACE[p]);
      ret->data.owner = ret;
      //return &CURSOR_SPACE[p];
      return ret;
   }
   
   //return p;
}

/**************************************
 *******free existed node to free list*
 *************************************/
//Position CursorFree(Position p)
void CursorFree(hashMapElement* ptr)
{
   Position p,prev;
   /*
   if(IsFirst(p,usedListHeader))
   {
      usedListHeader = CURSOR_SPACE[p].next;
   }
   else
   {
      prev = FindPrevious(p,usedListHeader);
      CURSOR_SPACE[prev].next = CURSOR_SPACE[p].next;
   }*/
   p = ptr->no;
   Delete(p,&usedListHeader,&usedListTail);
   //Reset(p);
   Insert(p,&freeListHeader,&freeListTail);
   //return p;
}


void InitializeCursorSpace()
{
   int i;
   for(i = 0;i <= POOL_SPACE_SIZE - 1;i++)
   {
      CURSOR_SPACE[i].no = i;
      CURSOR_SPACE[i].memPoolNext = i + 1;
      CURSOR_SPACE[i].key = 0;
      CURSOR_SPACE[i].hashMapNext = NULL;
   }
   //CURSOR_SPACE[POOL_SPACE_SIZE - 1].no = POOL_SPACE_SIZE - 1;
   CURSOR_SPACE[POOL_SPACE_SIZE - 1].memPoolNext = -1;
   
   freeListHeader = 0;
   freeListTail = POOL_SPACE_SIZE - 1;
   usedListHeader = -1;
   usedListTail = -1;
}


bool IsEmpty(Position listHeader)
{
   return listHeader == -1;
}

bool IsFirst(Position p,Position listHeader)
{
   return p == listHeader;
}

bool IsLast(Position p)
{
   return CURSOR_SPACE[p].memPoolNext == -1;
}

/*
Position Find(Addr_t addr,Position list)
{
   Position p;
   p = list;
   while(p != -1)
   {
      if(IsSameAddr(addr,CURSOR_SPACE[p].addr))
         return p;
      p = CURSOR_SPACE[p].next;
   }
   return -1;
}*/

Position FindPrevious(Position p,Position listHeader)
{
   Position tmp;
   tmp = listHeader;
   while(tmp != -1)
   {
      if(CURSOR_SPACE[tmp].memPoolNext == p)
         return tmp;
      tmp = CURSOR_SPACE[tmp].memPoolNext;
   }
   return -1;
      
}

void Delete(Position p,Position* listHeader,Position* listTail)
{
   Position prevP;
   if(IsFirst(p,*listHeader))//only when IsFirst, will cause the list empty
   {
      *listHeader = CURSOR_SPACE[p].memPoolNext;
      if(IsEmpty(*listHeader))
         *listTail = -1;
   }
   else
   {
      prevP = FindPrevious(p,*listHeader);
      CURSOR_SPACE[prevP].memPoolNext = CURSOR_SPACE[p].memPoolNext;
      if(IsLast(p))
         *listTail = prevP;
   }

   /*
   if(IsEmpty(*listHeader))
      *listTail = -1;
   return p;*/
}

void Insert(Position p,Position* listHeader,Position* listTail)
{
   if(IsEmpty(*listHeader))
   {
      *listHeader = *listTail = p;
   }
   else
   {
      CURSOR_SPACE[p].memPoolNext = CURSOR_SPACE[*listTail].memPoolNext;
      CURSOR_SPACE[*listTail].memPoolNext = p;
      *listTail = p;
   }
}

/*
void Reset(Position p)
{
   memset(&(CURSOR_SPACE[p].addr),0,sizeof(Addr_t));
   memset(CURSOR_SPACE[p].messages,0,sizeof(CURSOR_SPACE[p].messages));
}*/



/*
bool IsSameAddr(Addr_t one,Addr_t other)
{
   return one.src == other.src && one.dst == other.dst && one.srcPort == other.srcPort && one.dstPort == other.dstPort;
}*/

/*
hashMapElement* Retrive(Position p)
{
   return &(CURSOR_SPACE[p]);
}*/