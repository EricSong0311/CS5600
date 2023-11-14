/*
* message.c / Memory Hierarchy Simulation - Part I
*
* Chenyu Song / CS5600 / Northeastern University
* Fall 2023 / Oct 30, 2023
*
*/

#include "message.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <limits.h>

extern int cacheCount;

/**
 * 获取当前时间戳（毫秒级）。
 *
 * 返回值:
 * - long long: 当前时间的毫秒级时间戳。
 */
long long current_timestamp_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long milliseconds = spec.tv_sec * 1000LL + spec.tv_nsec / 1000000; // 将秒和纳秒转换为毫秒
    return milliseconds;
}

/**
 * 使用随机替换策略从缓存中移除一个条目。
 *
 * 参数:
 * - cacheHashTable[]: CacheHashEntry指针数组，指向缓存哈希表。
 * - hashTableSize: 整型，表示哈希表的大小。
 *
 * 返回值:
 * - int: 被替换的缓存条目的键。如果没有可替换的条目，返回-1。
 */
int randomReplacement(CacheHashEntry *cacheHashTable[], int hashTableSize) {
    int randomIndex = rand() % hashTableSize; // 随机选择一个哈希桶索引
    CacheHashEntry *entry = cacheHashTable[randomIndex];
    CacheHashEntry *prevEntry = NULL;

    if (entry == NULL) {
        // 如果选中的哈希桶是空的，则遍历找到非空的一个
        for (int i = 0; i < hashTableSize; i++) {
            if (cacheHashTable[i] != NULL) {
                randomIndex = i;
                entry = cacheHashTable[i];
                break;
            }
        }
    }

    if (entry != NULL) {
        // 随机选择桶内的一个条目进行替换
        int entriesInBucket = 0;
        CacheHashEntry *temp = entry;
        while (temp != NULL) {
            entriesInBucket++;
            temp = temp->next;
        }

        int randomEntryIndex = rand() % entriesInBucket;
        for (int i = 0; i < randomEntryIndex; i++) {
            prevEntry = entry;
            entry = entry->next;
        }

        // 执行替换
        int replacedKey = entry->key;
        if (prevEntry == NULL) {
            cacheHashTable[randomIndex] = entry->next;
        } else {
            prevEntry->next = entry->next;
        }
        free(entry);
        cacheCount--; // 减少缓存计数
        printf("随机替换的消息ID：%d 已从缓存中移除\n", replacedKey);
        return replacedKey;
    }

    return -1; // 如果缓存为空，则没有可替换的条目
}

/**
 * 使用最近最少使用(LRU)策略从缓存中移除一个条目。
 *
 * 参数:
 * - cacheHashTable[]: CacheHashEntry指针数组，指向缓存哈希表。
 * - hashTableSize: 整型，表示哈希表的大小。
 *
 * 返回值:
 * - int: 被替换的缓存条目的键。如果没有可替换的条目，返回-1。
 */
int lruReplacement(CacheHashEntry *cacheHashTable[], int hashTableSize) {
    // 找到最近最少使用的缓存条目
    int lruKey = -1;
    time_t lruTime = LONG_MAX;
    CacheHashEntry *lruEntry = NULL;
    CacheHashEntry *lruPrevEntry = NULL;

    for (int i = 0; i < hashTableSize; i++) {
        CacheHashEntry *prev = NULL;
        CacheHashEntry *entry = cacheHashTable[i];

        while (entry != NULL) {
            if (entry->time_search < lruTime) {
                lruTime = entry->time_search;
                lruKey = entry->key;
                lruEntry = entry;
                lruPrevEntry = prev;
            }
            prev = entry;
            entry = entry->next;
        }
    }

    //这说明找到了
    if (lruEntry != NULL) {
        // 从哈希表中删除该条目
        if (lruPrevEntry == NULL) {
            cacheHashTable[lruKey % hashTableSize] = lruEntry->next;
        } else {
            lruPrevEntry->next = lruEntry->next;
        }
        free(lruEntry);
        cacheCount--; // 减少缓存计数
        printf("最近最少使用的消息ID：%d 已从缓存中移除\n", lruKey);
        return lruKey;

    }
    return -1; // 没有找到可替换的条目


}

/**
 * 创建并初始化一个新的消息实例。
 *
 * 参数:
 * - identifier: 整型，消息的唯一标识符。
 * - sender: 字符串，消息发送者的名字。
 * - receiver: 字符串，消息接收者的名字。
 * - content: 字符串，消息内容。
 * - delFlag: 整型，表示消息是否已经发送。
 * - limitsize: 整型，消息内容的最大允许大小。
 *
 * 返回值:
 * - Message*: 指向新创建的消息实例的指针。如果消息内容超过限制大小或内存分配失败，则返回NULL。
 */
Message* create_msg(int identifier, const char* sender, const char* receiver, const char* content, int delFlag, int limitsize) {
    Message* message = (Message*)malloc(sizeof(Message));
    if (message == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for Message.\n");
        return NULL;
    }

    if (sizeof(content)>limitsize){
        fprintf(stderr,"Error: Message size is too large.\n");
        free(message);
        return NULL;
    }
    if (message != NULL) {
        message->identifier = identifier;
        message->time_sent = current_timestamp_ms(); ;
        strncpy(message->sender, sender, sizeof(message->sender));
        message->sender[sizeof(message->sender) - 1] = '\0';
        strncpy(message->receiver, receiver, sizeof(message->receiver));
        message->receiver[sizeof(message->receiver) - 1] = '\0';
        strncpy(message->content, content, sizeof(message->content));
        message->content[sizeof(message->content) - 1] = '\0';
        message->delivered = delFlag;
    }
    return message;
}

/**
 * 将消息存储在缓存中，并在需要时执行替换策略。
 *
 * 参数:
 * - msg: 指向Message结构的指针，表示要存储的消息。
 * - cacheHashTable[]: CacheHashEntry指针数组，指向缓存哈希表。
 * - hashTableSize: 整型，表示哈希表的大小。
 * - repStrategy: 整型，表示使用的替换策略（0表示LRU，1表示随机）。
 */
void store_msg(const Message* msg, CacheHashEntry *cacheHashTable[], int hashTableSize, int repStrategy) {
    if (msg == NULL) {
        return;
    }

    int identifier = msg->identifier;
    int hashIndex = identifier % hashTableSize;

    // 创建新的缓存哈希条目
    CacheHashEntry *newCacheEntry = (CacheHashEntry*)malloc(sizeof(CacheHashEntry));
    if (newCacheEntry == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for CacheHashEntry.\n");
        return;
    }

    newCacheEntry->key = identifier;
    newCacheEntry->messageWithStatus = (MessageWithStatus){ .message = *msg, .hitStatus = 3 };
    newCacheEntry->time_search = current_timestamp_ms();
    newCacheEntry->next = NULL;

    if (cacheCount >= 16) {
        if(repStrategy==0){
            int replacedKey = lruReplacement(cacheHashTable, hashTableSize);
            if (replacedKey != -1) {
                printf("缓存已满，进行LRU替换，消息ID：%d 已被移除\n", replacedKey);
            }
        }else if(repStrategy==1){
            int replacedKey = randomReplacement(cacheHashTable, hashTableSize);
            if (replacedKey != -1) {
                printf("缓存已满，进行LRU替换，消息ID：%d 已被移除\n", replacedKey);
            }
        }

    }

    // 添加新条目到缓存哈希表
    cacheCount++; // 增加缓存计数
    newCacheEntry->next = cacheHashTable[hashIndex];
    cacheHashTable[hashIndex] = newCacheEntry;
    printf("消息ID：%d 添加到缓存，现在缓存中有：%d个message\n", newCacheEntry->key,cacheCount);

    // 将消息写入磁盘，需检查消息是否已存在于磁盘
    FILE* readFile = fopen("./messages.txt", "r");
    if (readFile != NULL) {
        char line[1024];
        bool exists = false;
        while (fgets(line, sizeof(line), readFile) != NULL) {
            int existingID;
            if (sscanf(line, "%d", &existingID) == 1 && existingID == identifier) {
                exists = true;
                break;
            }
        }
        fclose(readFile);

        // 如果消息不存在，则将其写入磁盘
        if (!exists) {
            FILE* writeFile = fopen("./messages.txt", "a");
            if (writeFile != NULL) {
                fprintf(writeFile, "%d %ld %s %s %s %d\n", msg->identifier, msg->time_sent,
                        msg->sender, msg->receiver, msg->content, msg->delivered);
                printf("消息ID：%d 添加到disk\n", msg->identifier);
                fclose(writeFile);
            } else {
                fprintf(stderr, "Error: Unable to open file messages.txt for writing.\n");
            }
        }
    } else {
        fprintf(stderr, "Error: Unable to open file messages.txt for reading.\n");
    }
}

/**
 * 从缓存或磁盘中检索消息，并根据需要更新缓存。
 *
 * 参数:
 * - identifier: 整型，要检索的消息的标识符。
 * - cacheHashTable[]: CacheHashEntry指针数组，指向缓存哈希表。
 * - hashTableSize: 整型，表示哈希表的大小。
 * - repStrategy: 整型，表示使用的替换策略（0表示LRU，1表示随机）。
 *
 * 返回值:
 * - MessageWithStatus*: 指向包含检索到的消息及其状态的结构的指针。如果消息未找到，返回一个状态为3的新结构。
 */
MessageWithStatus* retrieve_msg(int identifier, CacheHashEntry *cacheHashTable[], int hashTableSize, int repStrategy) {
    int hashIndex = identifier % hashTableSize;
    CacheHashEntry *current = cacheHashTable[hashIndex];

    // 在缓存哈希表中查找消息
    while (current != NULL) {
        if (current->key == identifier) {
            current->time_search = current_timestamp_ms();
            current->messageWithStatus.hitStatus=1;
            printf("在缓存中找到消息ID：%d\n", identifier);

            return &current->messageWithStatus;
        }
        current = current->next;
    }

    FILE* file = fopen("messages.txt", "r");
    if (file != NULL) {
        Message msg;
        char line[1000];
        while (fgets(line, sizeof(line), file) != NULL) {
            if (sscanf(line, "%d %ld %49s %49s %799s %d", &msg.identifier, &msg.time_sent, msg.sender, msg.receiver, msg.content, &msg.delivered) == 6) {
                if (msg.identifier == identifier) {
                    Message* retrieved_msg = create_msg(msg.identifier, msg.sender, msg.receiver, msg.content, msg.delivered, Context_limit);
                    retrieved_msg->time_sent = msg.time_sent;

                    fclose(file);
                    printf("在cache中未找到，在disk中找到messageID：%d的message\n", identifier);

                    // 再将信息放入缓存
                    store_msg(retrieved_msg, cacheHashTable, 16, repStrategy);


                    MessageWithStatus* msgStatus = (MessageWithStatus*)malloc(sizeof(MessageWithStatus));
                    if (msgStatus == NULL) {
                        exit(1); // 处理内存分配失败
                    }
                    *msgStatus = (MessageWithStatus){ .message = *retrieved_msg, .hitStatus = 2 };
                    free(retrieved_msg);
                    return msgStatus;
                }
            }
        }
        fclose(file);
    }

    // 如果也未在磁盘中找到，则返回新消息状态
    MessageWithStatus* newMsgStatus = (MessageWithStatus*)malloc(sizeof(MessageWithStatus));
    if (newMsgStatus == NULL) {
        fprintf(stderr,"Memory allocation failed.\n");
        // 处理内存分配失败
        return NULL;
    }
    *newMsgStatus = (MessageWithStatus){ .hitStatus = 3 };
    return newMsgStatus;
}



/**
 * 生成一个随机长度（1到10）的数字字符串。
 *
 * 返回值:
 * - char*: 指向新生成的字符串的指针。字符串以 '\0' 结尾。需要手动释放分配的内存。
 */
char* generateRandomNumberString(){
    int length = rand() % 10 + 1;
    // 字符串缓冲区，包括字符和字符串终止符 '\0'
    char* str = (char*)malloc((length + 1) * sizeof(char));
    // 随机数种子初始化
    srand((unsigned int)time(NULL));
    // 生成随机数字字符
    for (int i = 0; i < length; i++) {
        // 生成随机数字 0-9，并将其转化为字符 '0'-'9'
        str[i] = '0' + rand() % 10;
    }
    // 在字符串末尾添加字符串终止符 '\0'
    str[length] = '\0';

    return str;
}