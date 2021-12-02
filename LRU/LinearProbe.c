#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//#include "LinkedList.c"
#include "LinkedList.h"
#include "LinkedListStruct.h"

#define KEY char*
#define CAPACITY 10
#define VALUE struct node*

#define DEFAULT_LF 0.3
#define LINEAR_CONSTANT 17
#define MAX_CAPACITY 2

static struct HashTable *hashTable;
int capacity = 10;

void resizeTable();

struct HashTable {
    KEY *keys;
    struct node **values;
    int threshold;
    double loadFactor;
    int usedBuckets;
    int keyCount;
    int capacity;
} HashTable;

int normalizeIndex(unsigned int keyHash) {
    return keyHash % CAPACITY;
}

int probe(int x) {
    return LINEAR_CONSTANT * x;
}

unsigned long djb2(unsigned char *str) {
    unsigned long hash = 5381;
    int c;
    while (c = *str++){
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void initializeHashTable(double loadFactor) {
    //printf("Reached\n");
    hashTable = malloc(sizeof(HashTable));
    hashTable->keys = calloc(capacity, sizeof(char*));
    hashTable->values = calloc(capacity, sizeof(struct node*));
    hashTable->loadFactor = loadFactor;
    hashTable->threshold = (int)(CAPACITY * loadFactor);
    hashTable->capacity = capacity;
    hashTable->usedBuckets = 0;
    hashTable->keyCount = 0;
}

int gcd(int a, int b) {
    if (b == 0) return a;
    return gcd(b, a % b);
}

void increaseCapacity() {
    capacity = (capacity * 2) + 1;
    printf("Resizing......\n");
}

void adjustCapacity() {
    while (gcd(LINEAR_CONSTANT, capacity) != 1)
        capacity++;
}

struct node* insert(KEY key, char* value) {

    if (hashTable->usedBuckets >= MAX_CAPACITY)
    {
        printf("Cache Has Reached Maximum Capacity!\n");
        deleteLast();
        // return;
    }
    
    if (hashTable->threshold < hashTable->usedBuckets)
        resizeTable();
  
    unsigned int offset = normalizeIndex(djb2(key));
    //printf("Offset: %d\n", offset);
    for (unsigned int i = offset, x = 1; ; i = normalizeIndex(offset + probe(x++)))
    {
        hashTable->keys[i] = key;
        hashTable->keyCount++;
        hashTable->usedBuckets++;
        //printf("reached!\n");
        hashTable->values[i] = insertFirst(value);
        //printf("reached!\n");
        //displayAllNode();
        //printf("----------------------------------\n");
        break;
    }
}

void resizeTable() {
    //printf("Resizing...\n");
    increaseCapacity();
    adjustCapacity();

    KEY *oldKeyTable = hashTable->keys;
    VALUE *oldValueTable = hashTable->values;
    KEY *newKeyTable = calloc(capacity, sizeof(char*));
    VALUE *newValueTable = calloc(capacity, sizeof(struct node));

    hashTable->keys = newKeyTable;
    hashTable->values = newValueTable;
    hashTable->threshold = (int)(capacity * DEFAULT_LF);

    for (int i = 0; i < hashTable->capacity; i++)
    {
        if (oldKeyTable[i] != NULL)
            insert(oldKeyTable[i], oldValueTable[i]->data);
    }

    hashTable->capacity = capacity;
    free(oldKeyTable);
    free(oldValueTable);
}

struct node* get(KEY key) {

    unsigned int offset = normalizeIndex(djb2(key));
    // printf("Offset: %d\n", offset);
    for (unsigned int i = offset, x = 1; ; i = normalizeIndex(offset + probe(x++)))
    {
        //printf("Loop: %d\n", i);
        if (hashTable->keys[i] != NULL)
        {
            if (hashTable->keys[i] == key)
            {
                struct node *oldVal = hashTable->values[i];
                return oldVal;
            }
        } else {
            printf("Element not found!\n");
            break;
        }       
    }  
}

bool keyExists(KEY key) {
    unsigned int hashCode = normalizeIndex(djb2(key));
    if (hashTable->keys[hashCode] != NULL)
        return true;
    return false;
} 

void displayArrayElements() {

    for (int i = 0; i < capacity; i++)
    {
        //printf("NULL ITEM\n");
        if (hashTable->keys[i] == NULL)
        {
            printf("NULL ITEM\n");
        } else {
            printf("Socket: %s\n", hashTable->values[i]->data);
        }
        
    }

}

