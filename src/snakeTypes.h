#ifndef SNAKE_TYPES_H
#define SNAKE_TYPES_H

/*** next points forward (tail to head direction) ***/
typedef struct linkedList{ // snake nodes
    char value;
    int x, y;
    struct linkedList *next;
} node_t;

typedef struct playingField{
    int lengthX;
    int lengthY;
    char* field;
} playingField_t;
 
typedef struct snake{
    node_t *head, *tail;
    int direction;
    int length;
} snake_t;

#endif
