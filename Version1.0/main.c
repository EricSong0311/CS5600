/*
* main.c / Memory Hierarchy Simulation - Part I
*
* Chenyu Song, Shuhao Liu/ CS5600 / Northeastern University
* Fall 2023 / Oct 30, 2023
*
*/

#include <stdio.h>
#include "message.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//配置每个消息的大小和缓存中的消息数
int message_size=1024;
int cache_size=16;
int contextlimit=800;

CacheEntry cache[16];
HashEntry hashTable[16];



int main() {

    // Initialize cache and hash table
    for (int i = 0; i < cache_size; i++) {
        cache[i].identifier = -1;
        hashTable[i].key = -1;
        hashTable[i].value = -1;
    }


    //test function
    for (int i = 0; i < 20; i++) {
        sleep(1);

        Message* msg = create_msg(i, "s1", "r1",generateRandomNumberString(),0,contextlimit);

        printf("新建message时间：%ld,内容：%s\n",msg->time_sent,msg->content);
        store_msg(msg,cache,hashTable);
    }

    Message* r1 = retrieve_msg(1,cache,hashTable);
    Message* r2 = retrieve_msg(2,cache,hashTable);

    if (r1 != NULL) {
        printf("Retrieved Message 1: ");
        printf("Unique ID: %d Sender: %s Receiver: %s Content: %s\n",
               r1->identifier, r1->sender, r1->receiver, r1->content);
    } else {
        printf("Message 1 not found.\n");
    }

    if (r2 != NULL) {
        printf("Retrieved Message 2: ");
        printf("Unique ID: %d Sender: %s Receiver: %s Content: %s\n",
               r2->identifier, r2->sender, r2->receiver, r2->content);
    } else {
        printf("Message 2 not found.\n");
    }

    //free(msg);
    free(r1);
    free(r2);



    //指标计算
    int hits = 0;
    int misses = 0;

    for (int i = 0; i < 100; i++) {
        sleep(1);
        if (i%5==2){
            MessageWithStatus *msgWithStatus1 = retrieve_msg(3,cache,hashTable);
            MessageWithStatus *msgWithStatus2 = retrieve_msg(3,cache,hashTable);
        }
        else{
            int random_number = rand() % 20;
            MessageWithStatus *msgWithStatus = retrieve_msg(random_number,cache,hashTable);
        }
        //前面生成了20个，所以1000次查找的时候应该在20个里面查找
        //问题：应该生成多少个message？需不需要考虑查找的数据不再disk里的情况？
//        int random_number = rand() % 20;
//        MessageWithStatus *msgWithStatus = retrieve_msg(random_number,cache,hashTable);
//        if(msgWithStatus->hitStatus==1){
//            hits++;
//        }else{
//            misses++;
//        }
    }

    printf("Hits: %d\n", hits);
    printf("Misses: %d\n", misses);
    printf("Hit Rate: %.2f%%\n", (double)hits / (hits + misses) * 100);



    return 0;
}

