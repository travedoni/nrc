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

char **split_commands(char *line) 
{
	char **commands = malloc(MAX_ARGS * sizeof(char *));
	char *command;
	int i = 0;
	char *saveptr;

	command = strtok_r(line, ";", &saveptr);
	while (command != NULL) {
		commands[i++] = command;
		command = strtok_r(NULL, ";", &saveptr);
	}
	commands[i] = NULL;
	return commands;
}

char **split_operators(char *line)
{
	char **commands = malloc(MAX_ARGS * sizeof(char *));
	char *command;
	int i = 0;
	char *saveptr;

	command = strtok_r(line, "&&||", &saveptr);
	while (command != NULL) {
		commands[i++] = command;
		command = strtok_r(NULL, "&&||", &saveptr);
	}
	commands[i] = NULL;
	return commands;
}	

char **split_pipes(char *line) 
{
	char **commands = malloc(MAX_ARGS * sizeof(char *));
	char *command;
	int i = 0;
	char *saveptr;

	command = strtok_r(line, "|", &saveptr);
	while (command != NULL) {
		commands[i++] = command;
		command = strtok_r(NULL, "|", &saveptr);
	}
	commands[i] = NULL;
	return commands;
}

char *extract_commands(char *line)
{
	char *start = strchr(line, '{');
	char *end = strrchr(line, '}');
	if (start && end && end > start) {
		*end = '\0';
		return strdup(start + 1);
	}
	
	return NULL;
}
