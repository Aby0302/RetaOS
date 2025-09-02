#include <stdio.h>
#include <unistd.h>

// Main function
int main(int argc, char* argv[]) {
    (void)argc; (void)argv; // Unused parameters
    
    printf("Hello from user mode!\n");
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());
    
    // Wait for 3 seconds
    for (int i = 3; i > 0; i--) {
        printf("Shutting down... %d\n", i);
        sleep(1);
    }
    
    return 0;
}
