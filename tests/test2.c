#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define MAX_STR_LEN 100
int main(int argc, char *argv[]) {
    FILE *my_file = fopen("output2.txt", "w");
    int console_arg = atoi(argv[1]);
    char input_str[MAX_STR_LEN];
    read(0, input_str, MAX_STR_LEN);
    int input_arg = atoi(input_str);
    input_arg += 2;
    input_arg += console_arg;
    char output_str[MAX_STR_LEN];
    sprintf(output_str, "%d\n", input_arg);
    // printf("%d", input_arg);
    fprintf(my_file, "%s\n", output_str);
    write(1, output_str, strlen(output_str));
    // char my_eof = EOF;
    // write(1, &my_eof, 1);
    return 0;
}