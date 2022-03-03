#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define MAX_STR_LEN 100
int main(int argc, char *argv[]) {
    // FILE *my_file = fopen("output1.txt", "w");
    int console_arg = atoi(argv[1]);
    char input_str[MAX_STR_LEN];
    read(0, input_str, MAX_STR_LEN);
    fprintf(my_file, "%s\n", input_str);
    int input_arg = atoi(input_str);
    input_arg += 1;
    input_arg += console_arg;
    char output_str[MAX_STR_LEN];
    sprintf(output_str, "%d\n", input_arg);
    write(1, output_str, strlen(output_str));
    return 0;
}