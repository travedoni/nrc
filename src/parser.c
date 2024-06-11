#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "parser.h"
#include "variables.h" 

#define MAX_ARGS 64

char **parse_line(char *line) 
{
	char **args = malloc(MAX_ARGS * sizeof(char *));
	int i = 0;
	int in_quote = 0;
	char *start = line;

	while (*line) {
		if (*line == '\'') {
			in_quote = !in_quote;
			memmove(line, line + 1, strlen(line));
		} else if (!in_quote && (*line == ' ' || *line == '\t' || *line == '\n')) {
			*line = '\0';
			if (start != line) 
				args[i++] = strdup(start);

			start = line + 1;
		}
		line++;
	}
	if (start != line && *start != '\0') 
		args[i++] = strdup(start);

	args[i] = NULL;
	return args;
}	

char **expand_patterns(char **args) 
{
	glob_t globbuf;
	char **expanded_args = malloc(MAX_ARGS * sizeof(char *));
	int i, j = 0;
		
	for (i = 0; args[i] != NULL; i++) {
		if (strpbrk(args[i], "*?[") != NULL) {
			glob(args[i], 0, NULL, &globbuf);
			for (size_t k = 0; k < globbuf.gl_pathc; k++) {
				expanded_args[j++] = strdup(globbuf.gl_pathv[k]);
			}
			globfree(&globbuf);
		} else {
			expanded_args[j++] = strdup(args[i]);
		}
	}
	expanded_args[j] = NULL;
	return expanded_args;
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
