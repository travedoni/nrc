#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "parser.h"
#include "variables.h" 

#define MAX_ARGS 64

char *substitute_vars(char *arg) 
{
	if (arg[0] == '$') {
		char *var_name = arg + 1;
		if (var_name[0] == '#') {
			int length = get_var_length(var_name + 1);
			char *length_str = malloc(10);
			snprintf(length_str, 10, "%d", length);
			return length_str;
		} else if (var_name[0] == '"') {
			return concatenate_var(var_name + 1);
		} else {
			return get_subscript(var_name);
		}
	}

	return arg;
}

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
				args[i++] = substitute_vars(strdup(start));

			start = line + 1;
		}
		line++;
	}
	if (start != line && *start != '\0') 
		args[i++] = substitute_vars(strdup(start));

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

char *get_subscript(char *arg)
{
	char *open_paren = strchr(arg, '(');
	if (open_paren) {
		char *close_paren = strchr(open_paren, ')');
		if (close_paren) {
			*close_paren = '\0';
			int index = atoi(open_paren + 1) - 1;
			char *var_name = strndup(arg, open_paren - arg);
			char **value = get_var(var_name);
			if (value && index >= 0 && index < get_var_length(var_name)) {
				free(var_name);
				return strdup(value[index]);
			}
			free(var_name);
		}
	}
	
	return arg;
}


