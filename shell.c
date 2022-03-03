#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <stdlib.h>

#define MAX_COMMANDS_N 20
#define MAX_COMMAND_LINE_LEN 100


int transporter(int arg_start, int arg_end, char *argv[]) {
    pid_t last_pid;
    int com_ind = 1;
    unsigned end_flag = 0;
    int prev_pipe[2]; 
    while (!end_flag) {
        int new_arg_end = arg_start;
        while (new_arg_end < arg_end) {
            if (strcmp(argv[new_arg_end], "|") == 0) {
                break;
            }
            new_arg_end += 1;
        }
        char *arg_vect[new_arg_end - arg_start + 1];
        arg_vect[new_arg_end - arg_start] = NULL;
        int i = 0;
        while (arg_start != new_arg_end) {
            arg_vect[i] = argv[arg_start];
            i += 1;
            arg_start += 1;
        }
        if (arg_start == arg_end) {
            end_flag = 1;
        } else {
            arg_start += 1;
        }
        int cur_pipe[2];
        if (!end_flag) {
            pipe(cur_pipe);
        }
        if ((last_pid = fork()) == 0) {
            if (com_ind > 1) {
                // close(prev_pipe[1]);
                dup2(prev_pipe[0], 0);  
            }
            if (!end_flag) {
                close(cur_pipe[0]);
                dup2(cur_pipe[1], 1);
            }
            execvp(arg_vect[0], arg_vect);
        }
        if (!end_flag) {
            close(cur_pipe[1]);
        }
        if (com_ind > 1) {
            close(prev_pipe[0]);   
        }
        prev_pipe[0] = cur_pipe[0];
        prev_pipe[1] = cur_pipe[1];
        com_ind += 1;
    }
    int status;
    int last_status = 0;
    pid_t pid;
    while((pid = wait(&status)) != -1) {
        if (pid == last_pid) {
            last_status = WEXITSTATUS(status);
        }
    };
    return last_status;
}

int change_inoutput(int arg_ind, char *argv[]) {
    if (strcmp(argv[arg_ind], "<") == 0) {
        arg_ind += 1;
        char *file_name = argv[arg_ind];
        int fd = open(file_name, O_RDONLY);
        dup2(fd, 0);
        arg_ind += 1;
        return 1;
    } else if (strcmp(argv[arg_ind], ">") == 0) {
        arg_ind += 1;
        char *file_name = argv[arg_ind];
        int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC);
        dup2(fd, 1);
        arg_ind += 1;
        return 1;
    } else if (strcmp(argv[arg_ind], ">>") == 0) {
        arg_ind += 1;
        char *file_name = argv[arg_ind];
        int fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND);
        dup2(fd, 1);
        arg_ind += 1;
        return 1;
    } else return 0;
}

int command(int arg_start, int arg_end, char *argv[], int executed[]) {
    if (fork() == 0) {
        int status;
        if (strcmp(argv[arg_start], "(") == 0) {
            for (int i = arg_start; i < arg_end; i++) {
                free(argv[i]);
            }
            int success = executed[arg_start];
            exit(success);
        }
        int arg_ind = arg_start;
        if (change_inoutput(arg_ind, argv)) {
            arg_ind += 2;
            if (change_inoutput(arg_ind, argv)) {

                arg_ind += 2;
            }
            status = transporter(arg_ind, arg_end, argv);
            for (int i = arg_start; i < arg_end; i++) {
                free(argv[i]);
            }
            exit(status == 0 ? 1 : 0);
        } else {
            int arg_ind = arg_start;
            int new_arg_end = arg_end;
            while (arg_ind < arg_end) {
                if ((strcmp(argv[arg_ind], ">>") == 0) || (strcmp(argv[arg_ind], ">") == 0) || (strcmp(argv[arg_ind], "<") == 0)) {
                    new_arg_end = arg_ind;
                    if (change_inoutput(arg_ind, argv)) {
                        arg_ind += 2;
                        if (change_inoutput(arg_ind, argv)) {
                            arg_ind += 2;
                        }
                    }
                    break;
                }
                arg_ind += 1;
            }
            status = transporter(arg_start, new_arg_end, argv);
            for (int i = arg_start; i < arg_end; i++) {
                free(argv[i]);
            }
            exit(status == 0 ? 1 : 0);
        }
    }
    int st;
    wait(&st);
    return WEXITSTATUS(st);
}

int cond_exec_command(int arg_start, int arg_end, char *argv[], int executed[]) {
    int end_flag = 0, if_yes_flag = 0, if_no_flag = 0;
    int new_arg_end = arg_start;
    while (new_arg_end < arg_end) {
        if (strcmp(argv[new_arg_end], "&&") == 0) {
            if_yes_flag = 1;
            break;
        } else if (strcmp(argv[new_arg_end], "||") == 0) {
            if_no_flag = 1;
            break;
        }
        new_arg_end += 1;
    }
    if (new_arg_end == arg_end) {
        end_flag = 1;
    }
    int success = command(arg_start, new_arg_end, argv, executed);
    
    if (!end_flag) {
        if ((success && if_yes_flag) || (!success && if_no_flag)) {
            success = cond_exec_command(new_arg_end + 1, arg_end, argv, executed);
        }
    }
    return success;

}

int shell_command(int arg_start, int arg_end, char *argv[]) {
    int executed[MAX_COMMANDS_N];
    int new_arg_start = arg_start;
    int new_arg_end;
    int end_flag = 0;
    while (!end_flag) {
        while (!end_flag) {
            if (strcmp(argv[new_arg_start], "(") == 0) {
                new_arg_start += 1;
                break;
            }
            new_arg_start += 1;
            if (new_arg_start == arg_end) {
                end_flag = 1;
                break;
            }
        }
        if (end_flag) {
            break;
        }
        new_arg_end = new_arg_start;
        int opened = 1;
        while ((strcmp(argv[new_arg_end], ")") != 0) || (opened != 1)) {
            if (strcmp(argv[new_arg_end], "(") == 0) {
                opened += 1;
            }
            if (strcmp(argv[new_arg_end], ")") == 0) {
                opened -= 1;
            }
            new_arg_end += 1;
        }
        int st;
        if (fork() == 0) {
            int status = shell_command(new_arg_start, new_arg_end, argv);
            exit(status);
        }
        wait(&st);
        executed[new_arg_start - 1] = WEXITSTATUS(st);
        new_arg_start = new_arg_end;
    }
    end_flag = 0;
    int ser_flag = 0, bg_flag = 0;
    new_arg_end = arg_start;
    while (new_arg_end < arg_end) {
        if (strcmp(argv[new_arg_end], ";") == 0) {
            ser_flag = 1;
            break;
        } else if (strcmp(argv[new_arg_end], "&") == 0) {
            bg_flag = 1;
            break;
        }
        new_arg_end += 1;
    }
    if (new_arg_end + bg_flag + ser_flag == arg_end) {
        end_flag = 1;
    }
    int success = 0;
    if (bg_flag) {
        // if (fork() == 0) {
        //     int fd[2];
        //     fd[0] = open("/dev/null", O_RDONLY);
        //     fd[1] = open("/dev/null", O_WRONLY);
        //     dup2(fd[0], 0);
        //     dup2(fd[1], 1);
        //     signal(SIGINT, SIG_IGN);
        //     cond_exec_command(arg_start, new_arg_end, argv, executed);
        //     exit(0);
        // }
        if (fork() == 0) {
            if (fork() == 0) {    
                int fd[2];
                fd[0] = open("/dev/null", O_RDONLY);
                fd[1] = open("/dev/null", O_WRONLY);
                dup2(fd[0], 0);
                dup2(fd[1], 1);
                signal(SIGINT, SIG_IGN);
                int success = cond_exec_command(arg_start, new_arg_end, argv, executed);
                exit(success);
            } else {
                exit(0);
            }
        } else {
            wait(NULL);
        }
    } else {
        success = cond_exec_command(arg_start, new_arg_end, argv, executed);
    }
    if (!end_flag) {
        shell_command(new_arg_end + 1, arg_end, argv);
    }
    return success;
}

char **get_argv(char *command_line, char *argv[], int *n_args) {
    int i = 0;
    int end_flag = (i == strlen(command_line));
    int arg_ind = 0;
    while (!end_flag) {
        while (command_line[i] == ' ') {
            i += 1;
            if (i == strlen(command_line)) {
                end_flag = 1;
                break;
            }
        }
        if (end_flag) {
            arg_ind -= 1;
            break;
        }
        int arg_start = i;
        int arg_end;
        while (command_line[i] != ' ') {
            i += 1;
            if (i == strlen(command_line)) {
                end_flag = 1;
                break;
            }
        }
        arg_end = i;
        argv[arg_ind] = (char *) malloc(arg_end - arg_start + 1);
        for (int j = arg_start; j < arg_end; j++) {
            argv[arg_ind][j - arg_start] = command_line[j]; 
        }
        argv[arg_ind][arg_end - arg_start] = '\0';
        arg_ind += 1;
    }
    *n_args = arg_ind;
    return argv;
}

int main() {
    while (1) {
        char command_line[MAX_COMMAND_LINE_LEN + 1];
        fgets(command_line, MAX_COMMAND_LINE_LEN, stdin);
        if (strlen(command_line) != 0) {
            if (command_line[strlen(command_line) - 1] == '\n') {
                command_line[strlen(command_line) - 1] = '\0';
            }
        }
        int arg_ind;
        char *argv[MAX_COMMANDS_N];
        get_argv(command_line, argv, &arg_ind);
        shell_command(0, arg_ind, argv);
    }
    return 0;
}
