#include <stdio.h> // all
#include <stdlib.h> // all
#include <string.h> // playfield and keyboardreader 
#include <time.h>   // main and place food
#include <pthread.h> // main and keyboardreader
#include <termios.h> // for keyboardreader
#include <unistd.h>  //for keyboardreader
#include <mqueue.h>

#include "snakeConstants.h"
#include "snakeTypes.h"

/*******TO DO ***************/
// bytte ut printf med print
// improve msg passing, msg reading and printing. maybe another thread
// improve direction functionality (full rework)
// make the whole board including boarder into nodes with values to identify which part of the boarder the snake goes through. (replace all ifs used now with a switch with the value of the boarder node as parameter)
// fix boarder, margin and start message
// updating single units worse performance than reprinting whole board?
// when moving horizontally make the editPlayingFieldCell print two dashes (--) to make the snake look better (maybe this gives false impression of how long the snake is? Find a better solution)

void initActionMq(mqd_t *mqDescriptor, int openFlags){
    struct mq_attr attr;

    attr.mq_maxmsg = 2;
    attr.mq_msgsize = sizeof(char);
    attr.mq_flags = 0; // blocking

    *mqDescriptor = mq_open ("/actions", openFlags, 0666, &attr);
    if (*mqDescriptor == (mqd_t)-1) {
        perror("Mq opening failed");
        exit(-1);
    }
}

void readActionMq (char *action, mqd_t *mqDescriptor) {
    ssize_t numBytesReceived = 0;
    //receive an int from mq
    numBytesReceived = mq_receive (*mqDescriptor, action, sizeof(char), NULL);
  //if (numBytesReceived == -1)
    //perror ("mq_receive failure");
}

void writeActionMq(char *action, mqd_t *mqDescriptor){
    if (mq_send(*mqDescriptor, action, sizeof(char) , 1) == -1)
        perror("mq_send failure");
}

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

void printPlayingField(){
    char *spaces = malloc((2*FIELD_LENGTH_X-1)*sizeof(char));
    char *topBoarder = malloc((2*FIELD_LENGTH_X+1)*sizeof(char));
    char *bottomBoarder = malloc((2*FIELD_LENGTH_X+1)*sizeof(char));
    char *margin = malloc((OFFSET_COL-2)*sizeof(char));

    memset(spaces, ' ', (2*FIELD_LENGTH_X-1)*sizeof(char));
    memset(topBoarder, 'L', (2*FIELD_LENGTH_X+1)*sizeof(char));
    memset(bottomBoarder, 'T', (2*FIELD_LENGTH_X+1)*sizeof(char));
    memset(margin, ' ', (OFFSET_COL-2)*sizeof(char));

    printf("\033[%i;%iH%s \n%sN", OFFSET_ROW-1/*row*/, OFFSET_COL-1/*col*/, topBoarder, margin);
    int i;
    for(i = FIELD_LENGTH_Y; i > 0; --i){
        printf("%s", spaces);
        printf("M \n%sN", margin);
    }
    printf("%s\b \n", bottomBoarder);
    
    free(spaces);
    free(topBoarder);
    free(bottomBoarder);
    free(margin);
}

/*
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
*/
int editPlayingFieldCell(playingField_t *PF, node_t *node){
    if((0 <= node->x && node->x < PF->lengthX) && (0 <= node->y && node->y < PF->lengthY)){
        PF->field[node->x + (node->y*PF->lengthX)] = node->value;
        printf("\033[%i;%iH%c\n", OFFSET_ROW+node->y/*row*/, OFFSET_COL+2*node->x/*col*/, node->value);
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
            node->x = snake->head->x > 0 ? snake->head->x-1 : FIELD_LENGTH_X-1;
            node->y = snake->head->y;
            break;
        case 2: // RIGHT
            node->x = snake->head->x < FIELD_LENGTH_X-1 ? snake->head->x+1 : 0;
            node->y = snake->head->y;
            break;
        case 3: // UP
            node->x = snake->head->x;
            node->y = snake->head->y > 0 ? snake->head->y-1 : FIELD_LENGTH_Y-1;
            break;
        case 4: // DOWN
            node->x = snake->head->x;
            node->y = snake->head->y < FIELD_LENGTH_Y-1 ? snake->head->y+1 : 0;
            break;
    }

}

int checkNewPosition(playingField_t *PF, node_t *node){ 
      char newPositionValue;
      if((0 <= node->x && node->x < PF->lengthX) && 
        (0 <= node->y && node->y < PF->lengthY)){
        newPositionValue = getPlayingFieldCellValue(PF, node->x, node->y);
        if(newPositionValue == '-' || newPositionValue == '|')
            return PART_OF_SNAKE;
        else if(newPositionValue=='+')
            return FOOD;
        else
            return EMPTY;
    }else{
        return EMPTY;//OUT_OF_FIELD; FIX THIS AND SWITCH INSIDE "moveSnake" FUNCTION
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
    head->value = '|';
    head->x = startX;
    head->y = startY-2;
    head->next = NULL;

    node_t *mid = malloc(sizeof(node_t));
    mid->value = '|';
    mid->x = startX;
    mid->y = startY-1;
    mid->next = head;

    node_t *tail = malloc(sizeof(node_t));
    tail->value = '|';
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
    node->value = snake->direction==UP || snake->direction==DOWN ? '|' : '-';
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
    char keyPressed;
    char lastMessage = 'w';
    
    // message queue variables
    mqd_t actionMqWriter;
    int status; 
    
    // sleep time setting
    struct timespec tim;
    tim.tv_sec = 0;
    tim.tv_nsec = 100000000L;
    

    static struct termios oldt, newt;
    
    pthread_mutex_t *actionLock = (pthread_mutex_t*)vargp;
    char *threadID = malloc(10);
    strcpy(threadID, "reader");
    
    initActionMq(&actionMqWriter, O_WRONLY);
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

    /*I choose 'q' to end input. Notice that EOF is also turned off
    in the non-canonical mode*/
    while((keyPressed=getchar())!= 'q'){
        //if(c == 'a' || c == 'd' || c == 'w' || c == 's'){    
        switch(keyPressed){
            case 'a':
            case 'd':
                if(lastMessage == 'w' || lastMessage == 's')
                    break;
                else
                    continue;
            case 'w':
            case 's':
                if(lastMessage == 'a' || lastMessage == 'd')
                    break;
                else
                    continue;
            default:
                continue;
        }
        writeActionMq(&keyPressed, &actionMqWriter);
        lastMessage = keyPressed;
        nanosleep(&tim, NULL);
    }
    writeActionMq(&keyPressed, &actionMqWriter);
    
    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    
    mq_close(actionMqWriter);
    pthread_exit((void*)threadID);
}

void startSnakeGame(){
    struct timespec tim;
    tim.tv_sec = 0;
    tim.tv_nsec = 200000000L;
    
    mq_unlink("/actions");

    mqd_t actionMqReader;
    initActionMq(&actionMqReader, O_CREAT | O_RDONLY | O_NONBLOCK);

    char *threadPointer;
    
    int alive = 1;
    int playing = 1;
    char nextAction='e';

    srand(time(NULL));

    playingField_t *playingField = newPlayingField(FIELD_LENGTH_X, FIELD_LENGTH_Y);
    snake_t *snake = newSnake(FIELD_LENGTH_X/2,FIELD_LENGTH_Y-1); 
    
    printf("************** SNAKE **************\n");
    printf("Controls are WASD and 'q' to quit.\n");
    printf("Press ENTER to start\n");
    while( getchar() != '\n' );
    
    //pthread_mutex_t actionLock = PTHREAD_MUTEX_INITIALIZER;
    pthread_t keyboardReader;
    pthread_create(&keyboardReader, NULL, readKeyboard, NULL);
    
    printPlayingField(playingField); 
    putStartSnakeOnPlayingField(playingField, snake->tail);    
    placeFood(playingField, snake);
    do{   
        nanosleep(&tim, NULL);
        readActionMq(&nextAction, &actionMqReader);   
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
    mq_close(actionMqReader);
    mq_unlink("/actions");
    pthread_join(keyboardReader, (void**)&threadPointer);
    free(threadPointer); // free threadID allocated in readkeyboard()
    freeMemory(playingField, snake);
}
