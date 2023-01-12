Important Notice: Fall 2022 course project for CS214 at Rutgers University. Please follow Rutgers University Academic Integrity Policy.
_________________________________________________________________________________________________________________________________________________________________________

Description: The partial implementation of shell in linux using processes, signals, and data structures in C.
_________________________________________________________________________________________________________________________________________________________________________

Working(Just like original Shell):
- Reads input from the user by printing a prompt ">"
- Supports absolute and relative paths, supports cd command
- Prints "No such file or directory"if the file doesn't exist
- Prints "command not found" if user inputs invalid commands
- If the command ends in ampersand symbol, the program runs in the background
- If ampersand symbol is not specified, then the program runs in the foreground
- Each line of input is a job and a job id is specified
- Reaps all zombie processes and displayes what signal terminated that process
- Can run a suspended job in the background using the bg command
- ctrl+c sends SIGINT to the foreground job and any of its child processes
- ctrl-z sends SIGTSTP to the foreground job and any of its child processes
- SIGTSTP suspends a job until SIGCONT is received
- A process can be killed using kill command
- The shell exits upon typing "exit" or ctrl+D on empty input line
- The fg command runs a suspended or background job in the foreground
- The jobs command shows current jobs, job ids, current status, and the command itself
- Can add jobs in background using bg %# command, for example: bg %1
- Can kill jobs using kill %# command, for example: kill %1
- Supports the ls command
_________________________________________________________________________________________________________________________________________________________________________
