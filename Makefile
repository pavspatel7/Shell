shell: shell.c
	gcc -g -fsanitize=address -o shell shell.c