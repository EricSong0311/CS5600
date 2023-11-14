//
// Created by 宋晨宇 on 11/7/23.
//

#ifndef P1_MESSAGE_H
#define P1_MESSAGE_H
#include <sys/time.h>
#include <stdbool.h>

// 定义缓存大小和消息上下文限制
#define CACHE_SIZE 16
#define Context_limit 800


typedef struct Message {
    int identifier;
    time_t time_sent;
    char sender[100];
    char receiver[100];
    char content[800];
    int delivered;
} Message;

typedef struct MessageWithStatus{
    Message message;
    int hitStatus;  //1:表示在cache中找到；2:表示在磁盘中找到；3:表示没找到（查找的是新消息）
} MessageWithStatus;

//在基于哈希的缓存结构中表示一个条目。
typedef struct CacheHashEntry {
    int key; // Message identifier%hashsize
    MessageWithStatus messageWithStatus; // 消息及其状态
    time_t time_search; // 最近一次访问时间
    struct CacheHashEntry *next; // 指向链表中的下一个条目
} CacheHashEntry;

Message* create_msg(int unique_id, const char* sender, const char* receiver, const char* content, int delFlag,int limitsize);
char* generateRandomNumberString();
int randomReplacement(CacheHashEntry *cacheHashTable[], int hashTableSize);
int lruReplacement(CacheHashEntry *cacheHashTable[], int hashTableSize);
void store_msg(const Message* msg, CacheHashEntry *cacheHashTable[], int hashTableSize, int repStrategy);
MessageWithStatus* retrieve_msg(int identifier, CacheHashEntry *cacheHashTable[], int hashTableSize, int repStrategy);

#endif //P1_MESSAGE_H
