#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H

// Lock a directory to prevent modifications
void lock_directory(const char *path);

// Unlock a directory to allow modifications
void unlock_directory(const char *path);

#endif
