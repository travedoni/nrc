#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "shell.h"
#include "builtins.h"
#include "utils.h"

#define MAX_ARGS 64

void shell_loop()
{
	char *line = NULL;
	size_t len = 0;
	char **args;
	int status = 1;

	do {
		printf("nrc$ ");
		if (getline(&line, &len, stdin) == -1) {
			perror("getline");
			exit(EXIT_FAILURE);
		}
		args = parse_line(line);
		if (args[0] != NULL) {
			status = execute_command(args);
		}
		free(args);
	} while (status);

	free(line);
}

char **parse_line(char *line) 
{
	char **args = malloc(MAX_ARGS * sizeof(char *));
	char *arg;
	int i = 0;

	arg = strtok(line, " \t\r\n\a");
	while (arg != NULL) {
		if (arg[0] == '$') {
			char *value = get_var(arg + 1);
			if (value != NULL) 
				arg = value;

		}
		args[i++] = arg;
		arg = strtok(NULL, " \t\r\n\a");
	}
	args[i] = NULL;
	return args;
}

int execute_command(char **args) 
{
	if (args[0] == NULL) 
		return 1;

	int i;
	for (i = 0; i < num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) 
			return (*builtin_func[i])(args);
	}

	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0) {
		// child process
		if (execvp(args[0], args) == -1) 
			perror("rc");

		exit(EXIT_FAILURE);
	} else if (pid < 0 ) {
		// error forking
		perror("rc");
	} else {
		// parent process
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}	



