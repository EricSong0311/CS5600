#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define repStrategy 1 //0表示LRU策略；1表示随机策略

int cacheCount = 0;


// 定义缓存哈希表和主哈希表
CacheHashEntry* cacheHashTable[CACHE_SIZE];

int main() {
    // 初始化主哈希表
    for (int i = 0; i < CACHE_SIZE; i++) {
        cacheHashTable[i] = NULL; // 初始化缓存哈希表
    }


    // 测试代码
    for (int i = 0; i < 20; i++) {
        usleep(1000);
        // 假设 `generateRandomNumberString` 是一个有效的函数，用于生成随机字符串
        char* content = generateRandomNumberString(); // 生成消息内容
        Message* msg = create_msg(i, "s1", "r1", content, 0, Context_limit);

        if (msg != NULL) {
            printf("新建message ID:%d, 时间：%ld, 内容：%s\n", msg->identifier , msg->time_sent, msg->content);
            store_msg(msg, cacheHashTable, CACHE_SIZE, repStrategy);

            free(msg); // 释放消息内存
        }
        free(content); // 释放生成的字符串内存
    }

    // 检索并打印消息
    for (int i = 0; i < 2; i++) {
        MessageWithStatus* r = retrieve_msg(i, cacheHashTable, CACHE_SIZE, repStrategy);
        if (r != NULL && r->hitStatus != 3) {
            printf("Retrieved Message %d: ", i);
            printf("Unique ID: %d Sender: %s Receiver: %s Content: %s\n",
                   r->message.identifier, r->message.sender, r->message.receiver, r->message.content);
        } else {
            printf("Message %d not found.\n", i);
        }
    }

    // 统计命中率
    int hits = 0;
    int misses = 0;
    int test_set[1000];
    for (int i = 0; i < 1000; i++) {
        test_set[i] = rand() % 20;
    }

    for (int i = 0; i < 1000; i++) {
        usleep(1000);
        printf("随机访问消息ID：%d \n", test_set[i]);

        MessageWithStatus* msgWithStatus = retrieve_msg(test_set[i], cacheHashTable, CACHE_SIZE, repStrategy);
        if (msgWithStatus != NULL && msgWithStatus->hitStatus == 1) {
            hits++;
        } else {
            misses++;
        }
    }

    if(repStrategy==0){
        printf("LRU Hits: %d\n", hits);
        printf("LRU Misses: %d\n", misses);
        printf("LRU Hit Rate: %.2f%%\n", (double)hits / (hits + misses) * 100);
    } else if(repStrategy==1){
        printf("Random Hits: %d\n", hits);
        printf("Random Misses: %d\n", misses);
        printf("Random Hit Rate: %.2f%%\n", (double)hits / (hits + misses) * 100);
    }


    // 清理内存
    // [释放缓存和哈希表中的内存的代码]
    for (int i = 0; i < CACHE_SIZE; i++) {
        CacheHashEntry *currentEntry = cacheHashTable[i];
        while (currentEntry != NULL) {
            CacheHashEntry *nextEntry = currentEntry->next;
            free(currentEntry); // 释放 CacheHashEntry 对象
            currentEntry = nextEntry;
        }
    }


    return 0;
}
