#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h> // Include for stat
#include "log_manager.h"
#include "config.h"

void check_for_todays_department_uploads() {
    DIR *d;
    struct dirent *dir;
    struct stat file_stat; // Define a stat structure

    d = opendir(UPLOAD_DIR);
    if (d) {
        char today_date[11]; // Format: DD-MM-YYYY
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        snprintf(today_date, sizeof(today_date), "%02d-%02d-%04d", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);

        const char *departments[] = {"dept1", "dept2", "dept3", "dept4"};
        int files_found_for_dept[4] = {0};

        while ((dir = readdir(d)) != NULL) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, dir->d_name); // Construct full filepath
            if (stat(filepath, &file_stat) == 0) { // Use stat to get file info
                if (S_ISREG(file_stat.st_mode)) { // Check if it's a regular file
                    for (int i = 0; i < 4; i++) {
                        char expected_filename[256];
                        snprintf(expected_filename, sizeof(expected_filename), "%s_%s", departments[i], today_date);
                        if (strstr(dir->d_name, expected_filename) != NULL) {
                            files_found_for_dept[i] = 1;
                        }
                    }
                }
            }
        }
        closedir(d);

        for (int i = 0; i < 4; i++) {
            if (!files_found_for_dept[i]) {
                char log_msg[512];
                snprintf(log_msg, sizeof(log_msg), "No file found for %s on today's date: %s in %s", departments[i], today_date, UPLOAD_DIR);
                log_message(log_msg);
            }
        }
    } else {
        log_message("Failed to open UPLOAD_DIR for checking today's department uploads.");
    }
}

int main() {
    init_log_manager(LOG_FILE);
    check_for_todays_department_uploads();
    return 0;
}
