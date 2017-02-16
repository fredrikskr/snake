#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
static const int size = 10;
static const int offsetRow = 3;
static const int offsetCol = 3;

int main()
{
    char *spaces = malloc(size*sizeof(char));
    char *boarder = malloc((size+2)*sizeof(char));
    char *margin = malloc((offsetCol-1)*sizeof(char));
    
    memset(spaces, ' ', (2*size-1)*sizeof(char));
    memset(boarder, 'X', (2*size+1)*sizeof(char));
    memset(margin, ' ', (offsetCol-2)*sizeof(char));
    

    printf("\033[%i;%iH%s \n%sX", offsetRow-1/*row*/, offsetCol-1/*col*/, boarder, margin);
    int i;
    for(i = size; i > 0; --i){
        printf("%s", spaces);
        printf("X \n%sX", margin);
    }
    printf("%s\b \n", boarder);
    
    sleep(2);
 
    printf("\033[%i;%iHO O O O O O O O O O\n", offsetRow/*row*/, offsetCol/*col*/);

    sleep(2);

    printf("\033[%i;%iHO\n", offsetRow+5/*row*/, offsetCol+2*5/*col*/);
    
    free(spaces);
    free(boarder);
    free(margin);
    return 0;
}
