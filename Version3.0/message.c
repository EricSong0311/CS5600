/*
* message.c / Memory Hierarchy Simulation
*
* Chenyu Song / CS5600 / Northeastern University
* Fall 2023 / Nov 13, 2023
*
*/

#include "message.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>

extern int cacheCount;
extern LRUCache lruCache;

/**
  * Add an LRU node to the head of the LRU cache.
  *
  * Parameters:
  * - lruCache: Pointer to LRUCache structure, representing LRU cache.
  * - node: Pointer to the LRUNode structure, indicating the node to be added to the cache header.
  */
void addNodeToLRUHead(LRUCache *lruCache, LRUNode *node) {
    node->next = lruCache->head;
    node->prev = NULL;

    if (lruCache->head != NULL) {
        lruCache->head->prev = node;
    }
    lruCache->head = node;

    if (lruCache->tail == NULL) {
        lruCache->tail = node;
    }
}

/**
  * Remove a node from the LRU cache.
  *
  * Parameters:
  * - lruCache: Pointer to LRUCache structure, representing LRU cache.
  * - node: Pointer to the LRUNode structure, indicating the node to be removed from the cache.
  */
void removeNodeFromLRU(LRUCache *lruCache, LRUNode *node) {
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        lruCache->head = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        lruCache->tail = node->prev;
    }
}

/**
  * Move a node to the head of the LRU cache.
  *
  * Parameters:
  * - lruCache: Pointer to LRUCache structure, representing LRU cache.
  * - node: Pointer to the LRUNode structure, indicating the node to be moved.
  */
void moveToHead(LRUCache *lruCache, LRUNode *node) {
    removeNodeFromLRU(lruCache, node);
    addNodeToLRUHead(lruCache, node);
}


/**
  * Get the current timestamp in milliseconds.
  *
  * return value:
  * - long long: The current millisecond timestamp.
  */
long long current_timestamp_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    //Convert seconds and nanoseconds to milliseconds
    long long milliseconds = spec.tv_sec * 1000LL + spec.tv_nsec / 1000000;
    return milliseconds;
}

/**
  * Implement a random replacement policy to remove an entry from the cache.
  *
  * Parameters:
  * - cacheHashTable[]: cache hash table array.
  * - hashTableSize: Hash table size.
  * - lruCache: Pointer to LRUCache structure, representing LRU cache.
  * - cacheCount: Pointer to an integer representing the number of entries currently in the cache.
  *
  * return value:
  * - int: Key of the cache entry being replaced. If the cache is empty, -1 is returned.
  */
int randomReplacement(CacheHashEntry *cacheHashTable[], int hashTableSize, LRUCache *lruCache, int *cacheCount) {
    // cache is empty
    if (*cacheCount == 0) {
        return -1;
    }

    // Randomly select a hash bucket index
    srand((unsigned int)time(NULL));
    int hashIndex = rand() % hashTableSize;
    CacheHashEntry *current = cacheHashTable[hashIndex];
    CacheHashEntry *prev = NULL;

    // If the selected hash bucket is empty, traverse to find a non-empty one
    while (current == NULL) {
        hashIndex = (hashIndex + 1) % hashTableSize;
        current = cacheHashTable[hashIndex];
    }

    // Randomly select an entry within the selected bucket for replacement
    int entriesInBucket = 0;
    CacheHashEntry *temp = current;
    while (temp != NULL) {
        entriesInBucket++;
        temp = temp->next;
    }

    int randomEntryIndex = rand() % entriesInBucket;
    for (int i = 0; i < randomEntryIndex; i++) {
        prev = current;
        current = current->next;
    }

    // Perform replacement
    int replacedKey = current->key;
    removeNodeFromLRU(lruCache, current->lrNode);
    free(current->lrNode);

    if (prev == NULL) {
        cacheHashTable[hashIndex] = current->next;
    } else {
        prev->next = current->next;
    }

    free(current);
    (*cacheCount)--;

    printf("Randomly replaced message ID：%d has been removed from cache\n", replacedKey);
    return replacedKey;
}



/**
  * Remove an entry from the cache using the least recently used (LRU) policy.
  *
  * Parameters:
  * - cacheHashTable[]: cache hash table array.
  * - hashTableSize: Hash table size.
  * - lruCache: Pointer to LRUCache structure, representing LRU cache.
  * - cacheCount: Pointer to an integer representing the number of entries currently in the cache.
  *
  * return value:
  * - int: Key of the cache entry being replaced. If the LRU cache is empty, -1 is returned.
  */
int lruReplacement(CacheHashEntry *cacheHashTable[], int hashTableSize, LRUCache *lruCache, int *cacheCount) {
    if (lruCache->tail == NULL) {
        return -1;
    }

    LRUNode *lruNode = lruCache->tail;
    int replacedKey = lruNode->key;

    //Delete the last LRUNode and update the hash table
    int hashIndex = replacedKey % hashTableSize;
    CacheHashEntry *current = cacheHashTable[hashIndex];
    CacheHashEntry *prev = NULL;
    while (current != NULL) {
        if (current->key == replacedKey) {
            if (prev == NULL) {
                cacheHashTable[hashIndex] = current->next;
            } else {
                prev->next = current->next;
            }
            removeNodeFromLRU(lruCache, current->lrNode);
            free(current->lrNode);
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }

    (*cacheCount)--;

    printf("Least recently used message ID：%d has been removed from cache\n", replacedKey);
    return replacedKey;
}

/**
  * Create and initialize a new message instance.
  *
  * Parameters:
  * - identifier: integer, unique identifier of the message.
  * - sender: string, the name of the message sender.
  * - receiver: string, the name of the message receiver.
  * - content: string, message content.
  * - delFlag: Integer, indicating whether the message has been sent.
  * - limitsize: integer, the maximum allowed size of the message content.
  *
  * return value:
  * - Message*: Pointer to the newly created message instance. If the message content exceeds the limit size or memory allocation fails, NULL is returned.
  */
Message* create_msg(int identifier, const char* sender, const char* receiver, const char* content, int delFlag, int limitsize) {
    Message* message = (Message*)malloc(sizeof(Message));
    if (message == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for Message.\n");
        return NULL;
    }

    if (sizeof(content)>limitsize){
        fprintf(stderr,"Error: Message size is too large.\n");
        return NULL;
    }
    if (message != NULL) {
        message->identifier = identifier;
        message->time_sent = current_timestamp_ms();
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
  * Store messages in the cache and replace entries based on policy when the cache is full. At the same time, the message is saved to disk.
  *
  * Parameters:
  * - msg: Pointer to the Message structure, indicating the message to be stored.
  * - cacheHashTable[]: CacheHashEntry pointer array, pointing to the cache hash table.
  * - hashTableSize: integer, indicating the size of the hash table.
  * - lruCache: Pointer to LRUCache structure, used to maintain LRU information.
  * - cacheCount: Pointer to the number of entries in the cache.
  * - repStrategy: integer, indicating the replacement strategy used (0 means LRU, 1 means random).
  */
void store_msg(const Message* msg, CacheHashEntry *cacheHashTable[], int hashTableSize, LRUCache *lruCache, int *cacheCount, int repStrategy) {
    if (msg == NULL) {
        return;
    }

    int identifier = msg->identifier;
    int hashIndex = identifier % hashTableSize;

    CacheHashEntry *newCacheEntry = (CacheHashEntry*)malloc(sizeof(CacheHashEntry));
    if (newCacheEntry == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for CacheHashEntry.\n");
        return;
    }

    LRUNode *newNode = (LRUNode*)malloc(sizeof(LRUNode));
    if (newCacheEntry == NULL || newNode == NULL) {
        free(newCacheEntry);
        free(newNode);
        return;
    }

    newCacheEntry->key = identifier;
    newCacheEntry->messageWithStatus = (MessageWithStatus){ .message = *msg, .hitStatus = 3 };
    newCacheEntry->time_search = current_timestamp_ms();
    newCacheEntry->lrNode = newNode;
    newCacheEntry->next = NULL;

    newNode->key = identifier;
    newNode->prev = NULL;
    newNode->next = NULL;

    //If cache is full, execute replacement strategy
    if (*cacheCount >= CACHE_SIZE) {
        if(repStrategy == 0) {
            lruReplacement(cacheHashTable, hashTableSize, lruCache, cacheCount);
        } else if(repStrategy == 1) {
            randomReplacement(cacheHashTable, hashTableSize, lruCache, cacheCount);
        }
    }

    newCacheEntry->next = cacheHashTable[hashIndex];
    cacheHashTable[hashIndex] = newCacheEntry;
    (*cacheCount)++;
    addNodeToLRUHead(lruCache, newNode);
    printf("message ID：%d is added to cache\n", msg->identifier);

    // Write the message to disk, you need to check whether the message already exists on the disk
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
                printf("message ID：%d is added to the disk\n", msg->identifier);
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
  * Retrieve messages from cache or disk and update the cache as needed.
  *
  * Parameters:
  * - identifier: integer, identifier of the message to be retrieved.
  * - cacheHashTable[]: CacheHashEntry pointer array, pointing to the cache hash table.
  * - hashTableSize: integer, indicating the size of the hash table.
  * - lruCache: Pointer to LRUCache structure, used to maintain LRU information.
  * - cacheCount: Pointer to the number of entries in the cache.
  * - repStrategy: integer, indicating the replacement strategy used (0 means LRU, 1 means random).
  *
  * return value:
  * - MessageWithStatus*: Pointer to a structure containing the retrieved message and its status. If the message is not found, a new structure with status 3 is returned.
  */
MessageWithStatus* retrieve_msg(int identifier, CacheHashEntry *cacheHashTable[], int hashTableSize, LRUCache *lruCache, int *cacheCount, int repStrategy) {
    int hashIndex = identifier % hashTableSize;
    CacheHashEntry *current = cacheHashTable[hashIndex];

    //Find message in cache first
    while (current != NULL) {
        if (current->key == identifier) {
            current->time_search = current_timestamp_ms();
            current->messageWithStatus.hitStatus = 1;
            moveToHead(lruCache, current->lrNode);

            printf("Find message with ID：%d in cache\n", identifier);
            return &current->messageWithStatus;
        }
        current = current->next;
    }
    //Then search message in the disk
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

                    printf("Not found in cache, message with ID ：%d was found in disk\n", identifier);
                    //Load data from disk to cache
                    store_msg(retrieved_msg, cacheHashTable, hashTableSize, lruCache, cacheCount, repStrategy);

                    MessageWithStatus* msgStatus = (MessageWithStatus*)malloc(sizeof(MessageWithStatus));
                    if (msgStatus == NULL) {
                        fprintf(stderr,"Memory allocation failed.\n");
                        exit(1);
                    }
                    *msgStatus = (MessageWithStatus){ .message = *retrieved_msg, .hitStatus = 2 };
                    free(retrieved_msg);
                    return msgStatus;
                }
            }
        }
        fclose(file);
    }
    // not found on disk
    MessageWithStatus* newMsgStatus = (MessageWithStatus*)malloc(sizeof(MessageWithStatus));
    if (newMsgStatus == NULL) {
        fprintf(stderr,"Memory allocation failed.\n");
        return NULL;
    }
    *newMsgStatus = (MessageWithStatus){ .hitStatus = 3 };

    return newMsgStatus;

}

/**
  * Generate a numeric string of random length (1 to 10).
  *
  * return value:
  * - char*: Pointer to the newly generated string. The string ends with '\0'. The allocated memory needs to be released manually.
  */
char* generateRandomNumberString(){
    int length = rand() % 10 + 1;
    char* str = (char*)malloc((length + 1) * sizeof(char));
    srand((unsigned int)time(NULL));
    for (int i = 0; i < length; i++) {
        str[i] = '0' + rand() % 10;
    }
    str[length] = '\0';
    return str;
}