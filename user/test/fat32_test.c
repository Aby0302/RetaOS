#include <kernel/vfs.h>
#include <kernel/console.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#define BUF_SIZE 1024

void test_file_operations() {
    console_printf("=== Testing File Operations ===\n");
    
    // Test file creation and writing
    const char* test_file = "/test_fat32.txt";
    const char* test_data = "This is a test file for FAT32 filesystem.\n";
    
    // Create and write to a file
    int fd = vfs_open(test_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        console_printf("Failed to create test file\n");
        return;
    }
    
    ssize_t written = vfs_write(fd, test_data, strlen(test_data));
    if (written < 0) {
        console_printf("Failed to write to test file\n");
        vfs_close(fd);
        return;
    }
    vfs_close(fd);
    console_printf("Successfully wrote %d bytes to %s\n", written, test_file);
    
    // Read back the file
    fd = vfs_open(test_file, O_RDONLY);
    if (fd < 0) {
        console_printf("Failed to open test file for reading\n");
        return;
    }
    
    char buf[BUF_SIZE];
    ssize_t bytes_read = vfs_read(fd, buf, sizeof(buf) - 1);
    if (bytes_read < 0) {
        console_printf("Failed to read from test file\n");
        vfs_close(fd);
        return;
    }
    buf[bytes_read] = '\0';
    vfs_close(fd);
    
    console_printf("Read %d bytes from %s:\n%s\n", bytes_read, test_file, buf);
    
    // Verify the content
    if (strcmp(buf, test_data) != 0) {
        console_printf("Error: File content doesn't match!\n");
    } else {
        console_printf("File content verification successful!\n");
    }
    
    // Clean up
    if (vfs_unlink(test_file) < 0) {
        console_printf("Failed to delete test file\n");
    } else {
        console_printf("Test file deleted successfully\n");
    }
}

void test_directory_operations() {
    console_printf("\n=== Testing Directory Operations ===\n");
    
    const char* test_dir = "/test_dir";
    
    // Create a test directory
    if (vfs_mkdir(test_dir, 0755) < 0) {
        console_printf("Failed to create test directory\n");
        return;
    }
    console_printf("Created directory: %s\n", test_dir);
    
    // Create a test file in the directory
    char test_file[256];
    snprintf(test_file, sizeof(test_file), "%s/test_file.txt", test_dir);
    
    int fd = vfs_open(test_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        console_printf("Failed to create test file in directory\n");
        vfs_rmdir(test_dir);
        return;
    }
    vfs_close(fd);
    console_printf("Created test file: %s\n", test_file);
    
    // List directory contents
    console_printf("\nListing directory %s:\n", test_dir);
    vfs_node_t* dir = vfs_open_dir(test_dir);
    if (!dir) {
        console_printf("Failed to open directory\n");
        vfs_unlink(test_file);
        vfs_rmdir(test_dir);
        return;
    }
    
    vfs_dirent_t entry;
    int count = 0;
    while (1) {
        vfs_dirent_t* result = vfs_read_dir(dir);
        if (!result) break;
        console_printf("  %s (%s, %d bytes)\n", 
                      result->name, 
                      result->is_dir ? "DIR" : "FILE",
                      result->size);
        count++;
    }
    console_printf("Total entries: %d\n", count);
    
    // Clean up
    if (vfs_unlink(test_file) < 0) {
        console_printf("Failed to delete test file\n");
    } else {
        console_printf("Test file deleted successfully\n");
    }
    
    if (vfs_rmdir(test_dir) < 0) {
        console_printf("Failed to remove test directory\n");
    } else {
        console_printf("Test directory removed successfully\n");
    }
}

int main(int argc, char** argv) {
    console_printf("=== FAT32 Filesystem Test ===\n\n");
    
    // List root directory contents
    console_printf("Root directory contents:\n");
    vfs_node_t* root_dir = vfs_open_dir("/");
    if (!root_dir) {
        console_printf("Failed to open root directory\n");
        return 1;
    }
    
    vfs_dirent_t entry;
    int count = 0;
    while (1) {
        vfs_dirent_t* result = vfs_read_dir(root_dir);
        if (!result) break;
        console_printf("  %s (%s, %d bytes)\n", 
                      result->name, 
                      result->is_dir ? "DIR" : "FILE",
                      result->size);
        count++;
    }
    console_printf("Total entries: %d\n\n", count);
    
    // Run file operations test
    test_file_operations();
    
    // Run directory operations test
    test_directory_operations();
    
    console_printf("\n=== FAT32 Test Completed ===\n");
    return 0;
}
