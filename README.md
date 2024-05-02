# System Software Daemon Project
## This daemon project was created and executed on a Linux (Ubuntu) Virtual Machine.

- Implemented directory monitor for new files added or modified by user.
- Created automated scheduled operation on file directory - file existence checker, scheduled file transfer, locking or unlocking of directory & directory backup.
- Constructed a log file - executed and utilized by daemon, records actions occurred.
- Combined multiple code files as 3 executable files - using Makefile, all ran by Daemon.

### The daemon is created to:
- Identify new or modified xml reports and log details of who made the changes, generated as a text file report and stored on the server.
- Monitor uploads of xml report files by 11.30pm each night.
- Transfer and backup uploads to the reporting directory. This happens at 1am.
- Logged in the system, when file wasn't uploaded (checked by naming convention - which department uploaded and which did not)
- Applies lock during backup or transfer, no user will be able to modify any files.
- IPC setup allowing all processes to report in on completion of a task.
- Error logging and reporting included for all main tasks in the solution.


### Daemon program is capable off:
- Backing up and transfering uploads at any time.
- Start running at boot up becoming root.

Makefile created to manage the creation of the executables - combined multiple code files as 3 executable files - using Makefile, all ran by Daemon.
