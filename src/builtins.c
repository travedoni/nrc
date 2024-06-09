#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "builtins.h"
#include "utils.h"

char *builtin_str[] = {
	"set",
	"get",
	"exit",
	"cd",
	"echo",
	"lc"
};

int (*builtin_func[]) (char **) = {
	&shell_set,
	&shell_get,
	&shell_exit,
	&shell_cd,
	&shell_echo,
	&shell_lc
};

int num_builtins() 
{
	return sizeof(builtin_str) / sizeof(char *);
}

int shell_set(char **args) 
{
	if (args[1] == NULL || args[2] == NULL) {
		fprintf(stderr, "nrc: expected variable name and value\n");
		return 1;
	}
	set_var(args[1], args[2]);
	return 1;
}

int shell_get(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "nrc: expected variable name\n");
		return 1;
	}
	char *value = get_var(args[1]);
	if (value != NULL) {
		printf("%s\n", value);
	} else {
		fprintf(stderr, "nrc: variable not found\n");
	}

	return 1;
}	

int shell_exit(char **args) 
{
	return 0;
}

int shell_cd(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "nrc: expected argument to \"cd\"\n");
	} else {
		if (chdir(args[1]) != 0) 
			perror("nrc");

	}
	
	return 1;
}

int shell_echo(char **args)
{
	for (int i = 1; args[i] != NULL; i++) {
		printf("%s ", args[i]);
	}
	printf("\n");
	return 1;
}

int shell_lc(char **args) 
{
	pid_t pid = fork();
	if (pid == 0) {
		if (args[1] == NULL) {
			execlp("ls", "ls", NULL);
		} else {
			execvp("ls", args);
		}
		perror("nrc");
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		perror("nrc");
	} else {
		wait(NULL);
	}

	return 1;
}
