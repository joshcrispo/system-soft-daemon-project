#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "lock_manager.h"
#include "log_manager.h"
#include "config.h"
#include <sys/stat.h>

void lock_directory(const char *path)
{
    if (chmod(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0)
    {
        /*
        char log_message_content[512];
        snprintf(log_message_content, sizeof(log_message_content), "Failed to set directory %s for owner write and others read-only.", path);
        log_message(log_message_content, 4);
        */
    }
    else
    {
        /*
        char log_message_success[512];
        snprintf(log_message_success, sizeof(log_message_success), "Directory %s set for owner write and others read-only successfully.", path);
        log_message(log_message_success, 4);
        */
    }
}

void unlock_directory(const char *path)
{
    if (chmod(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
    {
        /* 
        char log_message_content[512];
        snprintf(log_message_content, sizeof(log_message_content), "Failed to restore permissions for directory %s.", path);
        log_message(log_message_content,4);
        */
    }
    else
    {
        /*
        char log_message_content[512];
        snprintf(log_message_content, sizeof(log_message_content), "Permissions restored for directory %s.", path);
        log_message(log_message_content,4);
        */
    }
}
