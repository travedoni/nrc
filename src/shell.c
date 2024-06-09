#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "shell.h"
#include "builtins.h"
#include "parser.h"
#include "variables.h"

#define MAX_ARGS 64

void shell_loop() 
{
	char *line = NULL;
	size_t len = 0;
	char **commands;
	int status = 1;

	do {
		printf("nrc$ ");
		if (getline(&line, &len, stdin) == -1) {
			perror("getline");
			exit(EXIT_FAILURE);
		}
		commands = split_operators(line);
		for (int i = 0; commands[i] != NULL; i++) {
			char **args = parse_line(commands[i]);
			if (args[0] != NULL) {
				status = execute_command(args);
			}
			free(args);

			// Handle && and || operators
			if (commands[i + 1] != NULL) {
				if (strstr(commands[i], "&&") != NULL && status != 1) {
					break; // skip the rest of commands after &&
				} else if (strstr(commands[i], "||") != NULL && status == 1) {
					break; // skip the rest of commands after ||
				}
			}
		}
		free(commands);
	} while (status);

	free(line);
}

int execute_command(char **args) 
{
	int i;

	if (args[0] == NULL) {
		// empty command
		return 1;
	}

	for (i = 0; i < num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}

	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		// Child process
		if (execvp(args[0], args) == -1) {
			perror("nrc");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// Error forking
		perror("nrc");
	} else {
		// Parent process
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
