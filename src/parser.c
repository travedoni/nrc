#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "variables.h" 

#define MAX_ARGS 64

char **parse_line(char *line) 
{
	char **args = malloc(MAX_ARGS * sizeof(char *));
	char *arg;
	int i = 0;

	arg = strtok(line, " \t\r\n\a");
	while (arg != NULL) {
		if (arg[0] == '$') {
			char *value = get_var(arg + 1);
			if (value != NULL) {
				arg = value;
			}
		}
		args[i++] = arg;
		arg = strtok(NULL, " \t\r\n\a");
	}
	args[i] = NULL;
	return args;
}

char **split_operators(char *line)
{
	char **commands = malloc(MAX_ARGS * sizeof(char *));
	char *command;
	int i = 0;

	command = strtok(line, "&&||");
	while (command != NULL) {
		commands[i++] = command;
		command = strtok(NULL, "&&||");
	}
	commands[i] = NULL;
	return commands;
}	
