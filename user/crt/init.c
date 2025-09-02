#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

// Simple init program
// This is the first user program that runs at system startup
// For now, it just starts a shell

int main(int argc, char* argv[]) {
    printf("RetaOS - A Simple Operating System\n");
    printf("init: Starting system...\n");
    
    // Start shell
    const char* shell_path = "/bin/sh";
    char* const shell_argv[] = { "sh", NULL };
    
    printf("init: Starting %s...\n", shell_path);
    
    while (1) {
        pid_t child = fork();
        
        if (child == 0) {
            // Child process
            execve(shell_path, shell_argv, NULL);
            
            // If execve fails
            printf("init: Failed to start %s!\n", shell_path);
            printf("init: Trying to start /bin/hello.elf...\n");
            
            // If shell fails to start, try hello.elf
            const char* hello_path = "/bin/hello.elf";
            char* const hello_argv[] = { "hello", NULL };
            execve(hello_path, hello_argv, NULL);
            
            // If we get here, both programs failed to start
            printf("init: Could not start any program!\n");
            _exit(1);
        } else if (child > 0) {
            // Parent process
            int status;
            pid_t terminated_pid = waitpid(-1, &status, 0);
            
            if (terminated_pid > 0) {
                printf("init: PID %d terminated with status: %d\n", terminated_pid, status);
                
                // If shell terminated, restart it
                if (terminated_pid == child) {
                    printf("init: Shell terminated, restarting...\n");
                }
            } else {
                printf("init: waitpid error\n");
            }
        } else {
            // Fork error
            printf("init: fork() failed!\n");
            // Wait a bit and try again
            sleep(5);
        }
    }
    
    // Buraya asla ulaşılmamalı
    while (1) {}
    return 0;
}
