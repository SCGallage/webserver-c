#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_BUFFER_SIZE 2048

FILE *filePtr;

#include "LinkedList.h"
#include "LinkedListStruct.h"

struct node *head = NULL;
struct node *tail = NULL;
struct node *current = NULL;

struct node* insertFirst(char* value) {
    struct node *link = malloc(sizeof(struct node));

    char inputBuffer[MAX_BUFFER_SIZE];
    link->data = malloc(MAX_BUFFER_SIZE);
    strcpy(link->data, value);
    printf("Inserting Element To Cache.....\n");
    if (head == NULL)
    {
        tail = link;
    }
    if (head != NULL)
    {
        head->previous = link;   
    }
    link->next = head;
    head = link;
    return link;
}

void deleteLast() {
    printf("Evicting Last Element From Cache.....\n");
    struct node *last = tail;
    last->previous->next = NULL;
    tail = last->previous;
}

void displayAllNode() {

    struct node *current = tail;
    while (current != NULL)
    {
        printf("Value: %s\n", current->data);
        current = current->previous;
    }
    
}

// void readFile(char* filePath, struct node *new_node) {
//     fread(new_node->data, 2048, 1, filePtr);
//     printf("%s\n", new_node->data);
// }


// int main() {

//     insertFirst("Sanka");
//     insertFirst("Devin");
//     displayAllNode();
//     deleteLast();
//     displayAllNode();

//     return 0;

// }