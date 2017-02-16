#include <stdio.h> // all
#include <stdlib.h> // all
#include <string.h> // playfield and keyboardreader 
#include <time.h>   // main and place food
#include <pthread.h> // main and keyboardreader
#include <termios.h> // for keyboardreader
#include <unistd.h>  //for keyboardreader

#include "snakeConstants.h"
#include "snakeTypes.h"

/*******TO DO ***************/
// bytte ut printf med print
// bruk strcpy for A printe kantene i printPF
// have one thread
// change to read and write mutex


/********* PLAYING FIELD ***********/
 
playingField_t * newPlayingField(int lengthX, int lengthY){
    playingField_t *playingField = malloc(sizeof(playingField_t));
    playingField->field = malloc(lengthX*lengthY*sizeof(char));
    playingField->lengthX = lengthX;
    playingField->lengthY = lengthY;
    playingField->field = malloc(lengthX*lengthY*sizeof(char));
    memset(playingField->field, ' ', lengthX*lengthY*sizeof(char));
    return playingField;
}

void printPlayingField(playingField_t *PF){
    int area = PF->lengthX * PF->lengthY;
    int position;
    for(position = PF->lengthX; position > 0; --position){
        printf("xx");
    }
    printf("x\nx");
    for(position = 0; position < area; position++){
        printf("%c ", PF->field[position]);
        if(position != 0 && ((position+1)%(PF->lengthX) == 0))
            printf("\bx\nx");
    }
    for(position = PF->lengthX; position > 0; --position){
        printf("xx");
    }
    printf("\n\n");
}

int editPlayingFieldCell(playingField_t *PF, node_t *node){
    if((0 <= node->x && node->x < PF->lengthX) && (0 <= node->y && node->y < PF->lengthY)){
        PF->field[node->x + (node->y*PF->lengthX)] = node->value;
        return 1;
    }
    else{
        printf("ERROR: Out of range. Pos: (%i, %i)\n", node->x, node->y);
        return 0;
    }
}

char getPlayingFieldCellValue(playingField_t *PF, int xPos, int yPos){
    return PF->field[xPos + (yPos*PF->lengthX)];
}

void putStartSnakeOnPlayingField(playingField_t *PF, node_t *snakeTail){
    while(snakeTail != NULL){
        editPlayingFieldCell(PF, snakeTail);
        snakeTail = snakeTail->next;
    } 
}

void calculateNodePosition(snake_t *snake, node_t *node){
    switch(snake->direction){
        case 1: // LEFT
            node->x = snake->head->x-1;
            node->y = snake->head->y;
            break;
        case 2: // RIGHT
            node->x = snake->head->x+1;
            node->y = snake->head->y;
            break;
        case 3: // UP
            node->x = snake->head->x;
            node->y = snake->head->y-1;
            break;
        case 4: // DOWN
            node->x = snake->head->x;
            node->y = snake->head->y+1;
            break;
    }

}

int checkNewPosition(playingField_t *PF, node_t *node){ 
    if((0 <= node->x && node->x < PF->lengthX) && 
        (0 <= node->y && node->y < PF->lengthY)){
        if(getPlayingFieldCellValue(PF, node->x, node->y)=='O')
            return PART_OF_SNAKE;
        else if(getPlayingFieldCellValue(PF, node->x, node->y)=='+')
            return FOOD;
        else
            return EMPTY;
    }else{
        return OUT_OF_FIELD;
    }
}


void placeFood(playingField_t *PF, snake_t *snake){
    node_t food = {'+', rand()%PF->lengthX, rand()%PF->lengthY, NULL};
    while(checkNewPosition(PF, &food) != EMPTY){
        food.x = rand()%PF->lengthX;
        food.y = rand()%PF->lengthY;
    }
    editPlayingFieldCell(PF, &food);   
}

/***********************************/

/******** SNAKE FUNCTIONS *********/


snake_t * newSnake(int startX, int startY){
    snake_t *snake = malloc(sizeof(snake_t));

    node_t *head = malloc(sizeof(node_t));
    head->value = 'O';
    head->x = startX;
    head->y = startY-2;
    head->next = NULL;

    node_t *mid = malloc(sizeof(node_t));
    mid->value = 'O';
    mid->x = startX;
    mid->y = startY-1;
    mid->next = head;

    node_t *tail = malloc(sizeof(node_t));
    tail->value = 'O';
    tail->x = startX;
    tail->y = startY;
    tail->next = mid;

    snake->head = head;
    snake->tail = tail;
    snake->direction = UP;
    snake->length = 3;

    return snake;
}

int moveSnake(playingField_t *PF, snake_t *snake){
    node_t *node = malloc(sizeof(node_t));
    node->value = 'O';
    static int newPositionStatus = 0;
    static node_t *nodeToBeDeleted;
    calculateNodePosition(snake, node);
    newPositionStatus = checkNewPosition(PF, node);
    switch(newPositionStatus){
        case 5: // OUT OF FIELD
            return 0;
        case 6: // EMPTY
            nodeToBeDeleted = snake->tail;
            nodeToBeDeleted->value = ' ';
            snake->tail = nodeToBeDeleted->next;
            editPlayingFieldCell(PF, nodeToBeDeleted);
            free(nodeToBeDeleted);
            break;
        case 7: // PART OF SNAKE
            if((node->x == snake->tail->x) && (node->y == snake->tail->y)){
                nodeToBeDeleted = snake->tail;
                snake->tail = nodeToBeDeleted->next;
                free(nodeToBeDeleted);
                break;
            }
            else{
                return 0;
            }
        case 8: // FOOD
                placeFood(PF, snake);
                ++snake->length;
            break;
    }
    
    node->next = NULL;
    snake->head->next = node;
    snake->head = node;
    
    editPlayingFieldCell(PF, node);
}   

void changeSnakeDirection(snake_t *snake, int newDirection){
    if(((snake->direction == UP || snake->direction == DOWN)
        && (newDirection == LEFT || newDirection == RIGHT))
        || ((snake->direction == LEFT || snake->direction == RIGHT)
        && (newDirection == UP || newDirection == DOWN)))
        snake->direction = newDirection;
}    
/*********************************/


// Frees up allocated memory
void freeMemory(playingField_t *PF, snake_t *snake){
    free(PF->field);
    free(PF);
    node_t *node = snake->tail;
    node_t *tmp;
    int y;
    while(node != NULL){
        tmp = node;
        node = tmp->next;
        y=tmp->y;
        free(tmp);
    }
    free(snake);
}

char action = 'e';

void *readKeyboard(void *vargp){
    char c;   
    int placeInQueue;
    static struct termios oldt, newt;
    pthread_mutex_t *actionLock = (pthread_mutex_t*)vargp;
    char *threadID = malloc(10);
    strcpy(threadID, "reader");
    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr( STDIN_FILENO, &oldt);
    /*now the settings will be copied*/
    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON | ECHO);          

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    /*This is your part:
    I choose 'e' to end input. Notice that EOF is also turned off
    in the non-canonical mode*/
    while((c=getchar())!= 'q'){      
        if(c == 'a' || c == 'd' || c == 'w' || c == 's'){    
            if(action == 'e'){
                pthread_mutex_lock(actionLock);
                action = c;
                pthread_mutex_unlock(actionLock);
            }
        }
    }
    pthread_mutex_lock(actionLock);
    action = 'q';
    pthread_mutex_unlock(actionLock);       
    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    pthread_exit((void*)threadID);
}

void startSnakeGame(){
    struct timespec tim;
    tim.tv_sec = 0;
    tim.tv_nsec = 200000000L;


    pthread_mutex_t actionLock = PTHREAD_MUTEX_INITIALIZER;
    pthread_t keyboardReader;
    pthread_create(&keyboardReader, NULL, readKeyboard, &actionLock);
    char *threadPointer;
    
    int alive = 1;
    int playing = 1;
    char nextAction='e';

    srand(time(NULL));

    playingField_t *playingField = newPlayingField(FIELD_LENGTH_X, FIELD_LENGTH_Y);
    snake_t *snake = newSnake(FIELD_LENGTH_X/2,FIELD_LENGTH_Y-1); 
    putStartSnakeOnPlayingField(playingField, snake->tail);    
    placeFood(playingField, snake);
    
    printf("************** SNAKE **************\n");
    printf("Game starts in 2 seconds.\n");
    printf("Controls are WASD and 'q' to quit.\n");
    printf("Press 'w', 'a', 's' or 'd' to start\n");
    while(nextAction=='e'){
        pthread_mutex_lock(&actionLock);
        nextAction = action;
        pthread_mutex_unlock(&actionLock);    
    }
    pthread_mutex_lock(&actionLock);
    action = 'e';
    pthread_mutex_unlock(&actionLock);    
    
    
    do{   
        printPlayingField(playingField); 
        nanosleep(&tim, NULL);
        pthread_mutex_lock(&actionLock);
        nextAction = action;
        action = 'e';
        pthread_mutex_unlock(&actionLock);
        switch(nextAction){
            case 'a':
                changeSnakeDirection(snake, LEFT);
                break;
            case 'd':
                changeSnakeDirection(snake, RIGHT);
                break;
            case 'w':
                changeSnakeDirection(snake, UP);
                break;       
            case 's':
                changeSnakeDirection(snake, DOWN);
                break;       
            case 'q':
                playing = 0;
                break;
            default: 
                break;
        }
        alive = moveSnake(playingField, snake);
    }while(alive && playing);
    if(snake->length == playingField->lengthX*playingField->lengthY)
        printf("\tYOU WIN!!!!!!\n");
    else     
        printf("\tYOU LOSE!\n");
    printf("Press 'q' to quit.\n");

    /********** CLEAN UP ************/
    pthread_mutex_destroy(&actionLock);
    pthread_join(keyboardReader, (void**)&threadPointer);
    free(threadPointer); // free threadID allocated in readkeyboard()
    freeMemory(playingField, snake);
}
