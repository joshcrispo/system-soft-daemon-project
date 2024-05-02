#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "daemon_init.h"
#include "log_manager.h"
#include "file_monitor.h"
#include "transfer_manager.h"
#include "lock_manager.h"
#include "config.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

volatile sig_atomic_t stop;
pid_t file_monitor_pid = -1;

void inthandler(int signum)
{
    stop = 1;

    if (file_monitor_pid != -1)
    {
        kill(file_monitor_pid, SIGTERM);
    }
}

void urgent_change_signal_handler(int signum)
{
    if (signum == SIGUSR2)
    {
        chmod(REPORTING_DIR, S_IRWXU | S_IRWXG | S_IROTH);
        log_message("Write access granted to REPORTING_DIR.");
        alarm(300);
    }
}

void revert_permissions_handler(int signum)
{
    if (signum == SIGALRM)
    {
        chmod(REPORTING_DIR, S_IRUSR | S_IRGRP | S_IROTH);
        log_message("REPORTING_DIR reverted to read-only.");
    }
}

pid_t start_file_monitor()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        char errMsg[256];
        snprintf(errMsg, sizeof(errMsg), "fork failed: %s", strerror(errno));
        log_message(errMsg);
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    { // Child process
        char *args[] = {"./file_monitor", NULL};
        execvp(args[0], args);

        // If execv fails, log the error.
        char errMsg[256];
        snprintf(errMsg, sizeof(errMsg), "execvp failed: %s", strerror(errno));
        log_message(errMsg);
        exit(EXIT_FAILURE);
    }
    return pid; // Parent process returns child PID
}

// Function to check if the current time matches the specified hour and minute
int check_time(int hour, int min)
{
    time_t now = time(NULL);
    struct tm *now_tm = localtime(&now);

    return now_tm->tm_hour == hour && now_tm->tm_min == min;
}

void ensure_directory_exists(const char *path, mode_t mode)
{
    struct stat st = {0};

    if (stat(path, &st) == -1)
    {
        if (mkdir(path, mode) == -1)
        {
            char log_message_content[512];
            snprintf(log_message_content, sizeof(log_message_content), "Failed to create directory %s.", path);
            log_message(log_message_content);
        }
        else
        {
            char log_message_content[512];
            snprintf(log_message_content, sizeof(log_message_content), "Directory %s created successfully.", path);
            log_message(log_message_content);
        }
    }
}

void backup_directories()
{

    chmod(BACKUP_DIR, S_IRWXU | S_IRWXG | S_IROTH);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // Create a unique backup directory for today's date and time
    char backup_reports_path[256];
    snprintf(backup_reports_path, sizeof(backup_reports_path), "%s/backup_reports_%02d-%02d-%04d_%02d-%02d-%02d",
             BACKUP_DIR, t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);

    // Create the backup directory
    if (mkdir(backup_reports_path, 0777) == -1)
    {
        perror("Failed to create backup directory");
        exit(EXIT_FAILURE); // Or handle error appropriately
    }

    // Construct the command to copy the reports directory into the new backup directory
    char copy_command[1024];
    snprintf(copy_command, sizeof(copy_command), "cp -r %s %s/", REPORTING_DIR, backup_reports_path);

    system(copy_command);

    chmod(BACKUP_DIR, S_IRUSR | S_IRGRP | S_IROTH);
}

void signal_handler(int signum)
{
    if (signum == SIGUSR1)
    {
        backup_directories();
    }
}

void run_check_uploads_program()
{
    pid_t pid = fork();

    if (pid == -1)
    {
        // Error
        perror("fork");
    }
    else if (pid > 0)
    {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
    }
    else
    {
        // Child process
        char *args[] = {"./check_uploads", NULL};
        execvp(args[0], args);
        // execv only returns if an error occurred
        perror("execv");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    signal(SIGINT, inthandler);
    signal(SIGTERM, inthandler);
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, urgent_change_signal_handler);
    signal(SIGALRM, revert_permissions_handler);

    ensure_directory_exists(UPLOAD_DIR, 0755);
    ensure_directory_exists(REPORTING_DIR, 0755);
    ensure_directory_exists(BACKUP_DIR, 0755);

    // Set permissions
    chmod(REPORTING_DIR, S_IRUSR | S_IRGRP | S_IROTH);
    chmod(BACKUP_DIR, S_IRUSR | S_IRGRP | S_IROTH);

    daemonize();
    init_log_manager(LOG_FILE);
    log_message("Daemon started.");

    signal(SIGINT, inthandler);
    signal(SIGTERM, inthandler);

    // File monitoring in a separate process
    file_monitor_pid = start_file_monitor();

    // Previous minute tracker to prevent multiple executions in the same minute
    int prev_minute = -1;

    // Main loop
    while (!stop)
    {
        time_t now = time(NULL);
        struct tm *now_tm = localtime(&now);
        int minute_now = now_tm->tm_min;

        // Preventing multiple executions in the same minute
        if (prev_minute != minute_now)
        {
            prev_minute = minute_now;

            // Check if it's 1 AM
            if (check_time(1, 00))
            {
                log_message("Moving XML files to the reports folder.");
                lock_directory(UPLOAD_DIR);
                lock_directory(REPORTING_DIR);
                transfer_reports();
                backup_directories();
                unlock_directory(UPLOAD_DIR);
                unlock_directory(REPORTING_DIR);
                log_message("File moving completed.");
            }
            // Check if it's 11:30 PM
            if (check_time(23, 30))
            {
                run_check_uploads_program();
            }
        }

        // Sleep for a short time before checking again
        sleep(10); // Adjust sleep time as necessary
    }

    // Cleanup before exiting
    log_message("Daemon shutting down.");
    // Perform any necessary cleanup

    return 0;
}
