#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "log_manager.h"
#include <sys/file.h>

static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_log_manager(const char *log_file_path)
{
    pthread_mutex_lock(&log_mutex);
    if (log_file == NULL)
    {
        log_file = fopen(log_file_path, "a");
        if (log_file == NULL)
        {
            perror("Failed to open log file");
            exit(EXIT_FAILURE);
        }
    }
    pthread_mutex_unlock(&log_mutex);
}

void log_message(const char *message)
{
    pthread_mutex_lock(&log_mutex);
    if (log_file != NULL)
    {
        flock(fileno(log_file), LOCK_EX); // Lock the file

        // Get the current time
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[strlen(time_str) - 1] = '\0'; // Remove the newline

        // Write the log message with a timestamp
        fprintf(log_file, "[%s] %s\n ", time_str, message);
        fflush(log_file); // Flush the stream to ensure it's written

        flock(fileno(log_file), LOCK_UN); // Unlock the file
    }
    else
    {
        fprintf(stderr, "Log file is not initialized.\n");
    }
    pthread_mutex_unlock(&log_mutex);
}
