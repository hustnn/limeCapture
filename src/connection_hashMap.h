#ifndef __CONNECTION_HASHMAP_H
#define __CONNECTION_HASHMAP_H

#include "cursor_mem_pool.h"
#include "lime_capture.h"

#define MAP_OK 0

#define MAP_MISSING -3
#define MAP_ALEADY_ISHEADER -4

#define INITIAL_SIZE 1024

typedef void* any_t;

typedef any_t map_t;



typedef struct _hashMap_map
{
   int tableSize;
   int size;
   hashMapElement* table[INITIAL_SIZE];
}hashMap_map;



extern void HashMapInit();
extern void HashMapFree();
//extern int32_t HashMapHash(int32_t src,int32_t dst,u_int16_t srcPort,u_int16_t dstPort);
//extern int HashMapIndex(int32_t val);
extern int HashMapInsert(int key,hashMapElement* value);
extern int HashMapGet(Packet* packet,hashMapElement** arg);
extern int HashMapRemove(hashMapElement* value);

static int HashMapGetPrevious(hashMapElement* value,hashMapElement** arg);
#endif