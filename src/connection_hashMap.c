#include "connection_hashMap.h"
#include <math.h>


static hashMap_map* connHashTable;

void HashMapInit()
{
   int i;
   connHashTable = (hashMap_map*)malloc(sizeof(hashMap_map));

   if(!connHashTable)
      goto err;
   
   for(i = 0; i < INITIAL_SIZE; i++)
      connHashTable->table[i] = NULL;

   connHashTable->tableSize = INITIAL_SIZE;
   connHashTable->size = 0;
   //return m;
   return;

   err:
      if(connHashTable)
         HashMapFree();
      //return NULL;
}


void HashMapFree()
{
   //hashMap_map* m = (hashMap_map*)in;
   if(connHashTable)
      free(connHashTable);
}

/*
int32_t HashMap_hash(int32_t src,int32_t dst,u_int16_t srcPort,u_int16_t dstPort)
{
   int i;
   int32_t val = 0;
   double base = 31;
   double k;
   char tmp[ADDR_SIZE];
   char *s_src,*s_dst,*s_srcPort,*s_dstPort;
   s_src = (char*)(&src);
   s_dst = (char*)(&dst);
   s_srcPort = (char*)(&srcPort);
   s_dstPort = (char*)(&dstPort);
   memcpy(tmp,s_src,4);
   memcpy(tmp + 4,s_dst,4);
   memcpy(tmp + 8,s_srcPort,2);
   memcp(tmp+10,s_dstPort,2);
   for(i = 0;i < ADDR_SIZE;i++)
   {
      k = (double)(11 - i);
      val += tmp[i] * pow(base,k);
   }
   return val;
}*/

int HashMapInsert(int key,hashMapElement* value)
{
   int index;
   hashMapElement* prev;
   hashMapElement* cur = value;
   index = abs(key) % INITIAL_SIZE;
   prev = connHashTable->table[index];
   connHashTable->table[index] = cur;
   cur->hashMapNext = prev;
   return MAP_OK;
}

int HashMapGet(Packet* packet,hashMapElement** arg)
{
   int32_t key;
   u_int32_t index;
   hashMapElement* p;
   key = packet->hashPacket();
   index = abs(key) % INITIAL_SIZE;
   p = connHashTable->table[index];
   while(p != NULL)
   {
      if(p->key == key)
      {
         *arg = (hashMapElement*)p;
         return MAP_OK;
      }
      p = p->hashMapNext;
   }
   *arg = NULL;
   return MAP_MISSING;
}
/*
int HashMapGet(int32_t src,int32_t dst,u_int16_t srcPort,u_int16_t dstPort,hashMapElement** arg)
{
   int32_t key;
   int index;
   hashMapElement* p;
   key = HashMapHash(src,dst,srcPort,dstPort);
   index = key % INITIAL_SIZE;
   p = connHashTable.table[index];
   while(p != NULL)
   {
      if(p->key == key)
      {
         *arg = (hashMapElement*)p;
         return MAP_OK;
      }
      p = p->hashMapNext;
   }
   *arg = NULL;
   return MAP_MISSING;
}*/

int HashMapGetPrevious(hashMapElement* value,hashMapElement** arg)
{
   hashMapElement *prev,*cur;
   int32_t key;
   int index;
   //key = HashMapHash();
   key = value->data.iniPacket->hashPacket();
   index = abs(key) % INITIAL_SIZE;
   prev = NULL;
   cur = connHashTable->table[index];
   while(cur != NULL)
   {
      if(cur->key == value->key)
      {
         *arg = prev;
         if(prev == NULL)
            return MAP_ALEADY_ISHEADER;
         else
            return MAP_OK;
      }
      prev = cur;
      cur = cur->hashMapNext;
   }
   *arg = NULL;
   return MAP_MISSING;
}

int HashMapRemove(hashMapElement* value)
{
   int ret;
   hashMapElement* prev = NULL;
   ret = HashMapGetPrevious(value,&prev);
   if(ret == MAP_MISSING)
      return MAP_MISSING;
   else if(prev == NULL)
   {
      connHashTable->table[abs(value->key) % INITIAL_SIZE] = value->hashMapNext;
      value->hashMapNext = NULL;
   }
   else
   {
      prev->hashMapNext = value->hashMapNext;
      value->hashMapNext = NULL;
   }
   //CursorFree(value); delete from connection delete operator

   return MAP_OK;
}