#ifndef SNAKE_CONSTANTS_H
#define SNAKE_CONSTANTS_H

// SNAKE MOVE DIRECTION
static const int LEFT = 1;
static const int RIGHT = 2;
static const int UP = 3;
static const int DOWN = 4;

// MOVEMENT RESULTS
static const int OUT_OF_FIELD = 5;
static const int EMPTY = 6;
static const int PART_OF_SNAKE = 7;
static const int FOOD = 8; 

// FIELD SIZE
static const int FIELD_LENGTH_X = 10;
static const int FIELD_LENGTH_Y = 10;

// TERMINAL PRINT FORMATTING
static const int OFFSET_ROW = 3;
static const int OFFSET_COL = 3;

#endif
