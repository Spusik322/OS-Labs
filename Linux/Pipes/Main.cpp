#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

using namespace std;

int main() {
    int pipe_MA[2];
    int pipe_AP[2];
    int pipe_PS[2];
    
    if (pipe(pipe_MA) == -1 || pipe(pipe_AP) == -1 || pipe(pipe_PS) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    pid_t pidM, pidA, pidP, pidS;
    
    pidM = fork();
    if (pidM == 0) {
        close(pipe_MA[0]);
        dup2(pipe_MA[1], STDOUT_FILENO);
        close(pipe_MA[1]);
        
        execl("./M", "M", NULL);
        perror("execl M");
        exit(EXIT_FAILURE);
    }
    
    pidA = fork();
    if (pidA == 0) {
        close(pipe_MA[1]);
        close(pipe_AP[0]);
        dup2(pipe_MA[0], STDIN_FILENO);
        dup2(pipe_AP[1], STDOUT_FILENO);
        close(pipe_MA[0]);
        close(pipe_AP[1]);
        
        execl("./A", "A", NULL);
        perror("execl A");
        exit(EXIT_FAILURE);
    }
    
    pidP = fork();
    if (pidP == 0) {
        close(pipe_AP[1]);
        close(pipe_PS[0]);
        dup2(pipe_AP[0], STDIN_FILENO);
        dup2(pipe_PS[1], STDOUT_FILENO);
        close(pipe_AP[0]);
        close(pipe_PS[1]);
        
        execl("./P", "P", NULL);
        perror("execl P");
        exit(EXIT_FAILURE);
    }
    
    pidS = fork();
    if (pidS == 0) {
        close(pipe_PS[1]);
        dup2(pipe_PS[0], STDIN_FILENO);
        close(pipe_PS[0]);
        
        execl("./S", "S", NULL);
        perror("execl S");
        exit(EXIT_FAILURE);
    }

    close(pipe_MA[0]);
    close(pipe_MA[1]);
    close(pipe_AP[0]);
    close(pipe_AP[1]);
    close(pipe_PS[0]);
    close(pipe_PS[1]);
    
    waitpid(pidM, NULL, 0);
    waitpid(pidA, NULL, 0);
    waitpid(pidP, NULL, 0);
    waitpid(pidS, NULL, 0);
    
    return 0;
}