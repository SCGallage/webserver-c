#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_BUFFER_SIZE 2048

FILE *filePtr;

struct node
{
    struct node *previous;
    struct node *next;
    char *data;
} node;

struct node *head = NULL;
struct node *tail = NULL;
struct node *current = NULL;

struct node* insertFirst(char* value) {
    struct node *link = malloc(sizeof(struct node));

    char inputBuffer[MAX_BUFFER_SIZE];
    //link->data = value;
    link->data = malloc(MAX_BUFFER_SIZE);
    //filePtr = fopen(value, "rb");
    //fread(link->data, MAX_BUFFER_SIZE, 1, filePtr);
    // link->data = inputBuffer;
    // memset(inputBuffer, 0, 2048);
    strcpy(link->data, value);
    if (head == NULL)
    {
        //printf("Hi\n");
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