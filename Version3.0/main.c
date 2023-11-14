/*
* main.c / Memory Hierarchy Simulation
*
* Chenyu Song / CS5600 / Northeastern University
* Fall 2023 / Nov 13, 2023
*
*/

#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int cacheCount = 0;
CacheHashEntry* cacheHashTable[CACHE_SIZE];
LRUCache lruCache = {NULL, NULL};

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Please enter an argument.\n");
        return -1;
    }

    int repStrategy=-1;
    if(strcmp(argv[1], "0") == 0){
        repStrategy=0;
        printf("-----------------------------------------------Use LRU strategy-----------------------------------------------\n");
    }else if(strcmp(argv[1], "1") == 0){
        repStrategy=1;
        printf("-----------------------------------------------Use Random strategy-----------------------------------------------\n");
    }else{
        printf("The input argument is illegal, please enter 0 (representing LRU) or 1 (representing Random)");
        return -1;
    }

    // Initialize the hash table
    for (int i = 0; i < CACHE_SIZE; i++) {
        cacheHashTable[i] = NULL;
    }


    // Test code----Create 20 messages
    printf("---------------------------------------Create 20 messages---------------------------------------\n");
    for (int i = 0; i < 20; i++) {
        usleep(1000);
        char* content = generateRandomNumberString(); //Generate message content
        Message* msg = create_msg(i, "s1", "r1", content, 0, Context_limit);

        if (msg != NULL) {
            printf("New message ID:%d, Time：%ld, Content：%s\n", msg->identifier , msg->time_sent, msg->content);
            store_msg(msg, cacheHashTable, CACHE_SIZE, &lruCache, &cacheCount, repStrategy);
            free(msg);
        }
        free(content);
    }

    // Test code----retrieve and print 20 messages created before
    printf("---------------------------------------Retrieve 20 created messages---------------------------------------\n");
    for (int i = 0; i < 20; i++) {
        MessageWithStatus* r = retrieve_msg(i, cacheHashTable, CACHE_SIZE, &lruCache, &cacheCount, repStrategy);
        if (r != NULL && r->hitStatus != 3) {
            printf("Retrieved Message %d: ", i);
            printf("Unique ID: %d Sender: %s Receiver: %s Content: %s\n",
                   r->message.identifier, r->message.sender, r->message.receiver, r->message.content);
        } else {
            printf("Message %d not found.\n", i);
        }
    }

    // Statistical hit rate indicators
    int hits = 0;
    int misses = 0;
    int test_set[1000];
    for (int i = 0; i < 1000; i++) {
        test_set[i] = rand() % 20;
    }

    printf("---------------------------------------1000 random message accessess---------------------------------------\n");
    for (int i = 0; i < 1000; i++) {
        usleep(1000);
        printf("access message ID：%d \n", test_set[i]);

        MessageWithStatus* msgWithStatus = retrieve_msg(test_set[i], cacheHashTable, CACHE_SIZE, &lruCache, &cacheCount, repStrategy);
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

    // Free memory in cache and hash table
    for (int i = 0; i < CACHE_SIZE; i++) {
        CacheHashEntry *currentEntry = cacheHashTable[i];
        while (currentEntry != NULL) {
            CacheHashEntry *nextEntry = currentEntry->next;
            free(currentEntry);
            currentEntry = nextEntry;
        }
    }

    return 0;
}
