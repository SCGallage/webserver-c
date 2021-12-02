#ifndef LINKED_LIST_STRUCT
#define LINKED_LIST_STRUCT

struct node
{
    struct node *previous;
    struct node *next;
    char *data;
} node;

#endif