#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "config.h"
#include "log_manager.h"

//#define UPLOAD_DIR "/home/joshdaboyy/Documents/Assignment/uploads"
//#define REPORTING_DIR "/home/joshdaboyy/Documents/Assignment/reports"

void transfer_reports() {
    DIR *d;
    struct dirent *dir;
    char src_path[1024];
    char dest_path[1024];

    d = opendir(UPLOAD_DIR);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            // Skip "." and ".." entries
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
                continue;
            }

            // Construct the full source path using absolute path
            snprintf(src_path, sizeof(src_path), "%s/%s", UPLOAD_DIR, dir->d_name);

            // Construct the destination path using absolute path
            snprintf(dest_path, sizeof(dest_path), "%s/%s", REPORTING_DIR, dir->d_name);

            // Attempt to move the file without stat check
            if (rename(src_path, dest_path) == 0) {
                char log_message_content[1024];
                snprintf(log_message_content, sizeof(log_message_content), "File moved from %s to %s.", src_path, dest_path);
                log_message(log_message_content);
            } else {
                // If rename fails, log the failure
                char log_message_content[1024];
                snprintf(log_message_content, sizeof(log_message_content), "Failed to move file from %s to %s.", src_path, dest_path);
                log_message(log_message_content);
            }
        }
        closedir(d);
    } else {
        log_message("Failed to open UPLOAD_DIR for transfer.");
    }
    log_message("Report transfer operation complete.");
}
