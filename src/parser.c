#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "parser.h"
#include "variables.h"

#define MAX_ARGS 64

char *concatenate(char *left, char *right)
{
	size_t len_left = strlen(left);
	size_t len_right = strlen(right);
	char *result = malloc(len_left + len_right + 1);
	strcpy(result, left);
	strcat(result, right);
	return result;
}

char **concatenate_lists(char **left, char **right, int left_len, int right_len)
{
	char **result;
	int result_len = left_len > right_len ? left_len : right_len;
	result = malloc((result_len + 1) * sizeof(char *));

	if (left_len == right_len) {
		for (int i = 0; i < left_len; i++) {
			result[i] = concatenate(left[i], right[i]);
		}
	} else if (left_len == 1) {
		for (int i = 0; i < right_len; i++) {
			result[i] = concatenate(left[0], right[i]);
		}
	} else if (right_len == 1) {
		for (int i = 0; i < left_len; i++) {
			result[i] = concatenate(left[i], right[0]);
		}
	} else {
		fprintf(stderr, "nrc: invalid concatenation\n");
		free(result);
		return NULL;
	}

	result[result_len] = NULL;
	return result;
}

char **parse_concatenation(char *arg)
{
	char *caret = strchr(arg, '^');
	if (!caret) {
        return parse_line(arg);
	}

    // Concatenate args
	*caret = '\0';
	char *left_part = arg;
	char *right_part = caret + 1;

	char **left_list = parse_line(left_part);
	char **right_list = parse_line(right_part);

	int left_len = 0;
	int right_len = 0;
	while (left_list[left_len] != NULL) left_len++;
	while (right_list[right_len] != NULL) right_len++;

	char **result = concatenate_lists(left_list, right_list, left_len, right_len);

	free(left_list);
	free(right_list);

    // int result_len = 0;
    // for (int i = 0; result[i] != NULL; i++) {
    //     result_len += strlen(result[i]) + 1;
    // }

    // char *concatenate_str = malloc(result_len + 1);
    // concatenate_str[0] = '\0';
    // for (int i = 0; result[i] != NULL; i++) {
    //     strcat(concatenate_str, result[i]);
    //     if (result[i + 1] != NULL)
    //         strcat(concatenate_str, " ");
    // }

    // for (int i = 0; result[i] != NULL; i++) {
    //     free(result[i]);
    // }
    // free(result);

    // char **final_result = parse_line(concatenate_str);
    // free(concatenate_str);
	// return final_result;
    return result;
}

char *get_subscript(char *arg)
{
	char *open_paren = strchr(arg, '(');
	if (open_paren) {
		char *close_paren = strchr(open_paren, ')');
		if (close_paren) {
			*close_paren = '\0';

			char *subscript = open_paren + 1;
			while (*subscript == ' ') subscript++;
			char *end = close_paren - 1;
			while (end > subscript && *end == ' ') end --;
			*(end + 1) = '\0';

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
			char *result =  concatenate_var(var_name + 1);
			return result;
		} else if (strchr(var_name, '(')) {
			char *result = get_subscript(var_name);
			return result;
		} else {
			char **value = get_var(var_name);
			if (value) {
				char *result = concatenate_var(var_name);
				return result;
			}
		}
	}

	return strdup(arg);
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
		} else if (*line == '(') {
			*line = ' ';
			memmove(line, line + 1, strlen(line));
		} else if (*line == ')') {
			*line = '\0';
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
		commands[i++] = strdup(command);
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


