#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    int a;
    scanf("%d", &a);
    int n = atoi(argv[1]);
    n = n + 1 + a;
   printf("%d\n",n);
return 0;
}