#ifndef LOG_MANAGER_H_
#define LOG_MANAGER_H_

// Initializes the log manager.
void init_log_manager(const char* log_file_path);

// Logs a message to the specified log file.
void log_message(const char* message);

#endif
