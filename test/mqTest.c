#include <stdio.h>
#include <mqueue.h>

int main(int argc, char *argv[]){
    int x = 5;
    x = x==0 ? 1 : 2;
    printf("x = %i\n", x);
    return 0;
}
