#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 256
#define MAX_ARGS 16

// Command list
typedef struct {
    const char* name;
    const char* description;
    int (*func)(int argc, char* argv[]);
} command_t;

// Forward declaration of command table
static const command_t commands[];

// Help command
int cmd_help(int argc, char* argv[]) {
    (void)argc; (void)argv;
    printf("\nAvailable commands:\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        printf("  %-10s - %s\n", commands[i].name, commands[i].description);
    }
    printf("\nTo run external programs, specify the full path.\n");
    printf("Example: /bin/hello\n");
    return 0;
}

// Echo command
int cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    return 0;
}

// Clear command
int cmd_clear(int argc, char* argv[]) {
    (void)argc; (void)argv;
    printf("\033[2J\033[H"); // ANSI escape codes to clear the screen
    return 0;
}

// PS command - List processes
int cmd_ps(int argc, char* argv[]) {
    (void)argc; (void)argv;
    // This command will be handled specially by the kernel
    printf("  PID  CMD\n");
    printf("------------\n");
    // PS syscall
    asm volatile("int $0x80" : : "a" (39)); // SYS_ps = 39
    return 0;
}

// Memory information command
int cmd_meminfo(int argc, char* argv[]) {
    (void)argc; (void)argv;
    // This command will be handled specially by the kernel
    printf("Memory Information\n");
    printf("----------------\n");
    printf("Total:     16 MB\n");
    printf("Used:      4 MB\n");
    printf("Free:      12 MB\n");
    return 0;
}

// Command table
static const command_t commands[] = {
    {"help", "Shows help message", cmd_help},
    {"echo", "Prints input to the screen", cmd_echo},
    {"clear", "Clears the screen", cmd_clear},
    {"ps", "Lists running processes", cmd_ps},
    {"meminfo", "Shows memory information", cmd_meminfo},
    {NULL, NULL, NULL}
};

// Execute an external program
int execute_program(const char* path, int argc, char* argv[]) {
    int pid = fork();
    if (pid == 0) {
        // Child process - execute the program
        if (execve(path, argv, NULL) < 0) {
            printf("%s: failed to execute\n", path);
            _exit(1);
        }
    } else if (pid > 0) {
        // Parent process - wait for the program to finish
        int status;
        waitpid(pid, &status, 0);
        return status;
    } else {
        printf("Error: fork failed\n");
        return -1;
    }
    return 0;
}

// Execute a command
int execute_command(char* line) {
    char* args[MAX_ARGS];
    int argc = 0;
    
    // Split the command line into arguments
    char* token = strtok(line, " \t\n");
    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[argc] = NULL;
    
    if (argc == 0) {
        return 0; // Empty command
    }
    
    args[argc] = NULL;
    
    // Check internal commands
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(args[0], commands[i].name) == 0) {
            return commands[i].func(argc, args);
        }
    }
    
    // Try to execute external command
    // First look in /bin directory
    char path[256];
    snprintf(path, sizeof(path), "/bin/%s", args[0]);
    
    if (access(path, X_OK) == 0) {
        return execute_program(path, argc, args);
    }
    
    // If not found in /bin, try using the path directly
    if (access(args[0], X_OK) == 0) {
        return execute_program(args[0], argc, args);
    }
    
    printf("%s: command not found\n", args[0]);
    return 1;
}

// Main function
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    printf("RetaOS Shell\n");
    printf("Type 'help' for help\n\n");
    
    char input[MAX_INPUT];
    
    while (1) {
        printf("$ "); // Prompt
        
        // Get user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break; // EOF
        }
        
        // Check for exit command
        if (strncmp(input, "exit", 4) == 0) {
            break;
        }
        
        // Execute the command
        execute_command(input);
    }
    
    printf("Exiting...\n");
    return 0;
}
