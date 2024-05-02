all: build1 build2 build3

build1 : daemon_main.c daemon_init.c lock_manager.c log_manager.c transfer_manager.c
	gcc -o daemon_main daemon_main.c daemon_init.c lock_manager.c log_manager.c transfer_manager.c

build2: file_monitor.c log_manager.c
	gcc -o file_monitor file_monitor.c log_manager.c

build3: check_uploads.c log_manager.c
	gcc -o check_uploads check_uploads.c log_manager.c

run: 
	./daemon_main

clean: 
	rm -f daemon_main file_monitor check_uploads *.o

.PHONY: all run clean