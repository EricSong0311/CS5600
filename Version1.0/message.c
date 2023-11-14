/*
* message.c / Memory Hierarchy Simulation - Part I
*
* Chenyu Song, Shuhao Liu/ CS5600 / Northeastern University
* Fall 2023 / Oct 30, 2023
*
*/

#include "message.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

extern int contextlimit;

int randomReplacement(CacheEntry cache[], HashEntry hashTable[], int cacheSize, int hashTableSize) {
    // 寻找可替换的缓存条目
    int replaceIndex = -1;
    for (int i = 0; i < cacheSize; i++) {
        if (cache[i].identifier == -1) {
            replaceIndex = i;
            break;
        }
    }

    if (replaceIndex == -1) {
        // 所有缓存条目都被占用，需要随机选择一个进行替换
        replaceIndex = rand() % cacheSize;
    }

    // 获取要替换的缓存条目的关键
    int keyToReplace = cache[replaceIndex].identifier;

    // 从哈希表中删除对应的关键-值对
    for (int i = 0; i < hashTableSize; i++) {
        if (hashTable[i].key == keyToReplace) {
            hashTable[i].key = -1;  // 将关键设置为无效值，表示该位置为空
            break;
        }
    }

    printf("randomReplacement移除message ID:%d\n",cache[replaceIndex].identifier);
    // 清除要替换的缓存条目
    cache[replaceIndex].identifier = -1;
    cache[replaceIndex].time_sent = 0;
    memset(cache[replaceIndex].sender, 0, sizeof(cache[replaceIndex].sender));
    memset(cache[replaceIndex].receiver, 0, sizeof(cache[replaceIndex].receiver));
    memset(cache[replaceIndex].content, 0, sizeof(cache[replaceIndex].content));
    cache[replaceIndex].delivered = 0;

    return replaceIndex;  // 返回被替换的缓存条目的索引
}

int lruReplacement(CacheEntry cache[], HashEntry hashTable[], int cacheSize, int hashTableSize) {
    // 找到最近最少使用的缓存条目的索引
    int lruIndex = 0;
    time_t lruTime = cache[0].time_search;

    for (int i = 1; i < cacheSize; i++) {
        if (cache[i].time_search < lruTime) {
            lruTime = cache[i].time_search;
            lruIndex = i;
        }
    }

    // 从哈希表中找到要替换的条目的关键
    int keyToReplace = cache[lruIndex].identifier;
    printf("##############identifier ：%d被RLU移除，索引为%d：",cache[lruIndex].identifier,lruIndex);
    // 从哈希表中删除对应的关键-值对
    for (int i = 0; i < hashTableSize; i++) {
        if (hashTable[i].key == keyToReplace) {
            hashTable[i].key = -1;  // 将关键设置为无效值，表示该位置为空
            break;
        }
    }


    // 清空要替换的缓存条目
    memset(&cache[lruIndex], 0, sizeof(CacheEntry));
    return lruIndex;  // 返回被替换的缓存条目的索引
}


//LRU页面替换缓存中的一个条目
//int lruReplacement(CacheEntry cache[], HashEntry hashTable[], int cacheSize, int hashTableSize) {
//    // 找到最近最少使用的缓存条目的索引
//    int lruIndex = 0;
//    time_t lruTime = cache[0].time_search;
//
//    for (int i = 1; i < cacheSize; i++) {
//        if (cache[i].time_search < lruTime) {
//            lruTime = cache[i].time_search;
//            lruIndex = i;
//        }
//    }
//
//    // 从哈希表中找到要替换的条目的关键
//    int keyToReplace = cache[lruIndex].identifier;
//
//    // 从哈希表中删除对应的关键-值对
//    for (int i = 0; i < hashTableSize; i++) {
//        if (hashTable[i].key == keyToReplace) {
//            hashTable[i].key = -1;  // 将关键设置为无效值，表示该位置为空
//            break;
//        }
//    }
//
//    // 清空要替换的缓存条目
//    memset(&cache[lruIndex], 0, sizeof(CacheEntry));
//
//    return lruIndex;  // 返回被替换的缓存条目的索引
//}



Message* create_msg(int identifier, const char* sender, const char* receiver, const char* content, int delFlag, int limitsize) {
    Message* message = (Message*)malloc(sizeof(Message));

    if (sizeof(content)>limitsize){
        fprintf(stderr,"Error: Message size is too large.\n");
        return NULL;
    }
    if (message != NULL) {
        message->identifier = identifier;
        message->time_sent = time(NULL);
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

void store_msg(const Message* msg, CacheEntry cache[],HashEntry *hashTable[]) {
    if (msg == NULL) {
        return;
    }

    int identifier = msg->identifier;

    // Check if the cache is full
    int cache_index = -1;
    for (int i = 0; i < 16; i++) {
        if (cache[i].identifier == -1) {
            cache_index = i;
            printf("messageID：%d存储到cache索引为%d\n",identifier,cache_index);
            break;
        }
    }

    // Cache is full, perform page replacement (e.g., LRU)
    if (cache_index == -1) {
        // Implement LRU or random page replacement here
        //cache_index = randomReplacement(cache, hashTable, 16, 16);
        cache_index = lruReplacement(cache, hashTable, 16, 16);
        printf("cache中索引为%d的message被移出缓存,messageID：%d存储到缓存\n", cache_index, identifier);
    }


    // Store the message in the cache
    cache[cache_index].identifier = identifier;
    cache[cache_index].time_sent = msg->time_sent;
    strncpy(cache[cache_index].sender, msg->sender, sizeof(cache[cache_index].sender));
    cache[cache_index].sender[sizeof(cache[cache_index].sender) - 1] = '\0';
    strncpy(cache[cache_index].receiver, msg->receiver, sizeof(cache[cache_index].receiver));
    cache[cache_index].receiver[sizeof(cache[cache_index].receiver) - 1] = '\0';
    strncpy(cache[cache_index].content, msg->content, sizeof(cache[cache_index].content));
    cache[cache_index].content[sizeof(cache[cache_index].content) - 1] = '\0';
    cache[cache_index].delivered = msg->delivered;
    cache[cache_index].time_search=time(NULL);


    //添加或更新哈希表
    int hash_index = identifier % 16;
    HashEntry* newEntry = (HashEntry*)malloc(sizeof(HashEntry));
    newEntry->key = identifier;
    newEntry->value = cache_index;
    newEntry->next = NULL;

    if (hashTable[hash_index] == NULL) {
        hashTable[hash_index] = newEntry;
    } else {
        HashEntry* current = hashTable[hash_index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newEntry;
    }

    // Write to disk
    FILE* file = fopen("./messages.txt", "a");
    if (file != NULL) {
        fprintf(file, "%d %ld %s %s %s %d\n", msg->identifier, msg->time_sent,
                msg->sender, msg->receiver, msg->content, msg->delivered);
        printf("messageID：%d存储到disk中\n",identifier,cache_index);

        fclose(file);
    }
}


//MessageWithStatus* retrieve_msg(int identifier, CacheEntry cache[16],HashEntry hashTable[16]) {
//    int cache_index = -1;
////--------------------------------------------这个for循环能不能改成使用hashTable？时间复杂度降低
//    // Check the hash table for the cache entry index
//   // int hash_index = identifier % 16;
//    for (int i = 0; i<16 ; i++) {
//        if(identifier==cache[i].identifier){
//            cache_index=i;
//            printf("-----------查找数据-------- identifier：%d，cache_index：%d",identifier,cache_index);
//            break;
//        }
//    }
////    if (hashTable[hash_index].key == identifier) {
////        cache_index = hashTable[hash_index].value;
////    }
////    printf("-----------查找数据--------hashTable[hash_index].key:%d, identifier：%d，cache_index：%d",hashTable[hash_index].key,identifier,cache_index);
//
//    // If the cache entry index is valid, return the message
//    if (cache_index != -1) {
//        Message* msg = create_msg(cache[cache_index].identifier, cache[cache_index].sender, cache[cache_index].receiver, cache[cache_index].content,cache[cache_index].delivered, contextlimit);
//        msg->time_sent=cache[cache_index].time_sent;
//        cache[cache_index].time_search=time(NULL);
//
//        MessageWithStatus *msgWithStatus = (MessageWithStatus *)malloc(sizeof(MessageWithStatus));
//        if (msgWithStatus == NULL) {
//            // 处理内存分配失败的情况
//            exit(1); // 退出程序或采取适当的错误处理措施
//        }
//        msgWithStatus->message.identifier = msg->identifier; // 假设给定一个 identifier 值
//        msgWithStatus->message.time_sent = msg->time_sent; // 假设使用当前时间
//        strcpy(msgWithStatus->message.sender, msg->sender); // 假设设置发送者名称
//        strcpy(msgWithStatus->message.receiver, msg->receiver); // 假设设置接收者名称
//        strcpy(msgWithStatus->message.content, msg->content); // 假设设置消息内容
//        msgWithStatus->message.delivered = msg->delivered; // 假设消息已经被传送
//        msgWithStatus->hitStatus = 1;
//        printf("在cache中查找到messageID：%d的message，cache对应的索引是%d\n",identifier,cache_index);
//        return msgWithStatus;
//    }
//
//    // If the message is not in the cache, load it from disk
//    FILE* file = fopen("messages.txt", "r");
//    if (file != NULL) {
//        Message msg;
//        char line[1000];
//        while (fgets(line, sizeof(line), file) != NULL) {
//            if (sscanf(line, "%d %ld %49s %49s %255[^\n] %d", &msg.identifier, &msg.time_sent,
//                       msg.sender, msg.receiver, msg.content, &msg.delivered) == 5) {
//                if (msg.identifier == identifier) {
//                    Message* retrieved_msg = create_msg(msg.identifier, msg.sender, msg.receiver, msg.content, msg.delivered,contextlimit);
//                    retrieved_msg->time_sent = msg.time_sent;
//                    retrieved_msg->delivered = msg.delivered;
//                    fclose(file);
//                    printf("在cache中未找到，在disk中找到messageID：%d的message\n",identifier);
//
//                    MessageWithStatus *msgStatus = (MessageWithStatus *)malloc(sizeof(MessageWithStatus));
//                    if (msgStatus == NULL) {
//                        // 处理内存分配失败的情况
//                        exit(1); // 退出程序或采取适当的错误处理措施
//                    }
//                    msgStatus->message.identifier = retrieved_msg->identifier; // 假设给定一个 identifier 值
//                    msgStatus->message.time_sent = retrieved_msg->time_sent; // 假设使用当前时间
//                    strcpy(msgStatus->message.sender, retrieved_msg->sender); // 假设设置发送者名称
//                    strcpy(msgStatus->message.receiver, retrieved_msg->receiver); // 假设设置接收者名称
//                    strcpy(msgStatus->message.content, retrieved_msg->content); // 假设设置消息内容
//                    msgStatus->message.delivered = retrieved_msg->delivered; // 假设消息已经被传送
//                    msgStatus->hitStatus= 2;
//
//                    //再将信息放入缓存
//                    store_msg( retrieved_msg, cache, hashTable);
//                    return msgStatus;
//                }
//            }
//        }
//        fclose(file);
//    }
//    MessageWithStatus *msgStatus = (MessageWithStatus *)malloc(sizeof(MessageWithStatus));
//    if (msgStatus == NULL) {
//        // 处理内存分配失败的情况
//        exit(1); // 退出程序或采取适当的错误处理措施
//    }
//    msgStatus->hitStatus= 3;
//    //访问的是新消息，即没有在cache中，也没有在磁盘中
//    return msgStatus;
//
//}

MessageWithStatus* retrieve_msg(int identifier, CacheEntry cache[], HashEntry *hashTable[]) {
    int hash_index = identifier % 16; // 使用哈希函数计算索引
    HashEntry* current = hashTable[hash_index];

    // 遍历链表查找条目
    while (current != NULL) {
        if (current->key == identifier) {
            // 找到对应的缓存条目
            int cache_index = current->value;

            // 从缓存中创建消息对象
            Message* msg = create_msg(cache[cache_index].identifier, cache[cache_index].sender, cache[cache_index].receiver, cache[cache_index].content, cache[cache_index].delivered, contextlimit);
            msg->time_sent = cache[cache_index].time_sent;
            cache[cache_index].time_search = time(NULL);

            // 创建消息状态对象
            MessageWithStatus* msgWithStatus = (MessageWithStatus*)malloc(sizeof(MessageWithStatus));
            if (msgWithStatus == NULL) {
                // 处理内存分配失败的情况
                exit(1); // 退出程序或采取适当的错误处理措施
            }
            *msgWithStatus = (MessageWithStatus){ .message = *msg, .hitStatus = 1 };

            printf("在cache中查找到messageID：%d的message，cache对应的索引是%d\n", identifier, cache_index);
            return msgWithStatus;
        }
        current = current->next;
    }

    // 如果未找到，则从磁盘加载
    FILE* file = fopen("messages.txt", "r");
    if (file != NULL) {
        Message msg;
        char line[1000];
        while (fgets(line, sizeof(line), file) != NULL) {
            if (sscanf(line, "%d %ld %49s %49s %799s %d", &msg.identifier, &msg.time_sent, msg.sender, msg.receiver, msg.content, &msg.delivered) == 6) {
                if (msg.identifier == identifier) {
                    Message* retrieved_msg = create_msg(msg.identifier, msg.sender, msg.receiver, msg.content, msg.delivered, contextlimit);
                    retrieved_msg->time_sent = msg.time_sent;

                    fclose(file);
                    printf("在cache中未找到，在disk中找到messageID：%d的message\n", identifier);

                    // 再将信息放入缓存
                    store_msg(retrieved_msg, cache, hashTable);

                    MessageWithStatus* msgStatus = (MessageWithStatus*)malloc(sizeof(MessageWithStatus));
                    if (msgStatus == NULL) {
                        exit(1); // 处理内存分配失败
                    }
                    *msgStatus = (MessageWithStatus){ .message = *retrieved_msg, .hitStatus = 2 };
                    return msgStatus;
                }
            }
        }
        fclose(file);
    }

    // 访问的是新消息，即没有在cache中，也没有在磁盘中
    MessageWithStatus* msgStatus = (MessageWithStatus*)malloc(sizeof(MessageWithStatus));
    if (msgStatus == NULL) {
        exit(1); // 处理内存分配失败
    }
    *msgStatus = (MessageWithStatus){ .hitStatus = 3 };
    return msgStatus;
}


//生成1-10随机长度的数字字符串
char* generateRandomNumberString() {
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


