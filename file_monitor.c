#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <time.h>
#include "log_manager.h"
#include "config.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

void rename_if_needed(const char *fileName)
{
    // Simplified check for expected format: "deptX_DD-MM-YYYY.xml"
    if (strstr(fileName, "dept") == NULL || strstr(fileName, ".xml") == NULL)
    {
        // File name does not contain "dept" or ".xml", so rename it
        char newFileName[512];
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        snprintf(newFileName, sizeof(newFileName), "%s/unkno_%02d-%02d-%04d.xml", UPLOAD_DIR, t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);

        char oldFilePath[512], newFilePath[512];
        snprintf(oldFilePath, sizeof(oldFilePath), "%s/%s", UPLOAD_DIR, fileName);
        snprintf(newFilePath, sizeof(newFilePath), "%s", newFileName);

        if (rename(oldFilePath, newFilePath) == -1)
        {
            perror("Failed to rename file");
        }
        else
        {
            char logMessage[512];
            snprintf(logMessage, sizeof(logMessage), "Renamed file: %s to %s", fileName, newFileName);
            log_message(logMessage);
        }
    }
}

int main()
{
    init_log_manager(LOG_FILE);
    log_message("File Monitor started?");
    int fd, wd;
    char buffer[BUF_LEN];

    fd = inotify_init();
    if (fd < 0)
    {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    wd = inotify_add_watch(fd, UPLOAD_DIR, IN_MODIFY | IN_CREATE | IN_DELETE);

    while (1)
    {
        int length = read(fd, buffer, BUF_LEN);
        if (length < 0)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < length;)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                char log_message_content[512];
                if (event->mask & IN_CREATE)
                {
                    rename_if_needed(event->name);
                    snprintf(log_message_content, sizeof(log_message_content), "New file %s created in upload directory.", event->name);
                }
                else if (event->mask & IN_MODIFY)
                {
                    snprintf(log_message_content, sizeof(log_message_content), "File %s modified in upload directory.", event->name);
                }
                else if (event->mask & IN_DELETE)
                {
                    snprintf(log_message_content, sizeof(log_message_content), "File %s deleted from upload directory.", event->name);
                }
                log_message(log_message_content);
            }
            i += EVENT_SIZE + event->len;
        }
    }

    // Cleanup
    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}
