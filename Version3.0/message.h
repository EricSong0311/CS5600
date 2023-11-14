

#ifndef P1_MESSAGE_H
#define P1_MESSAGE_H
#include <sys/time.h>
#include <stdbool.h>

//Define cache size and message size
#define CACHE_SIZE 16
#define Message_limit 1024
#define Context_limit Message_limit-224

typedef struct Message {
    int identifier;
    time_t time_sent;
    char sender[100];
    char receiver[100];
    char content[Context_limit];
    int delivered;
} Message;

typedef struct MessageWithStatus{
    Message message;
    int hitStatus; //1: means found in cache; 2: means found on disk; 3: means not found (looking for new messages)
} MessageWithStatus;


typedef struct LRUNode {
    int key;
    struct LRUNode *prev;
    struct LRUNode *next;
} LRUNode;

typedef struct CacheHashEntry {
    int key; // Message identifier%hashsize
    MessageWithStatus messageWithStatus;
    LRUNode *lrNode;
    time_t time_search;
    struct CacheHashEntry *next;
} CacheHashEntry;


typedef struct {
    LRUNode *head;
    LRUNode *tail;
} LRUCache;


void addNodeToLRUHead(LRUCache *lruCache, LRUNode *node);
void removeNodeFromLRU(LRUCache *lruCache, LRUNode *node);
void moveToHead(LRUCache *lruCache, LRUNode *node);

long long current_timestamp_ms();
char* generateRandomNumberString();

int randomReplacement(CacheHashEntry *cacheHashTable[], int hashTableSize, LRUCache *lruCache, int *cacheCount);
int lruReplacement(CacheHashEntry *cacheHashTable[], int hashTableSize, LRUCache *lruCache, int *cacheCount);

Message* create_msg(int unique_id, const char* sender, const char* receiver, const char* content, int delFlag,int limitsize);
void store_msg(const Message* msg, CacheHashEntry *cacheHashTable[], int hashTableSize, LRUCache *lruCache, int *cacheCount, int repStrategy);
MessageWithStatus* retrieve_msg(int identifier, CacheHashEntry *cacheHashTable[], int hashTableSize, LRUCache *lruCache, int *cacheCount, int repStrategy);
#endif //P1_MESSAGE_H
