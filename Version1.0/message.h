//
// Created by Chengyu Song, Shuhao Liu on 11/7/23.
//

#ifndef P1_MESSAGE_H
#define P1_MESSAGE_H
#include <sys/time.h>
#include <stdbool.h>

typedef struct Message {
    int identifier;
    time_t time_sent;
    char sender[100];
    char receiver[100];
    char content[800];
    int delivered;
} Message;

typedef struct {
    Message message;
    int hitStatus;  //1:表示在cache中找到；2:表示在磁盘中找到；3:表示没找到（查找的是新消息）
} MessageWithStatus;

typedef struct CacheEntry {
    int identifier;  // 消息标识符
    time_t time_sent;
    char sender[100];  // 发件人
    char receiver[100];  // 收件人
    char content[1000];  // 消息内容
    int delivered;  // 是否已送达
    time_t time_search;//访问时间
} CacheEntry;


typedef struct HashEntry {
    int key;
    int value;
    struct HashEntry *next; // 指向链表中的下一个条目
} HashEntry;


int randomReplacement(CacheEntry cache[], HashEntry hashTable[], int cacheSize, int hashTableSize);
int lruReplacement(CacheEntry cache[], HashEntry hashTable[], int cacheSize, int hashTableSize);
Message* create_msg(int unique_id, const char* sender, const char* receiver, const char* content, int delFlag,int limitsize);
void store_msg(const Message* msg,CacheEntry cache[],HashEntry *hashTable[]);
//MessageWithStatus* retrieve_msg(int unique_id, CacheEntry cache[16],HashEntry hashTable[16]);
MessageWithStatus* retrieve_msg(int identifier, CacheEntry cache[], HashEntry *hashTable[]);

char* generateRandomNumberString();

#endif //P1_MESSAGE_H
