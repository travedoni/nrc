#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include "shell.h"
#include "builtins.h"
#include "parser.h"
#include "variables.h"

int process_command(char *line)
{
	char **commands;
	int status = 1;

	// Check if it's a variable assignment
	char *equals_sign = strchr(line, '=');
	if (equals_sign) {
		*equals_sign = '\0';
		char *var_name = line;
		char *var_value = equals_sign + 1;

		// Remove first and ending spaces
		var_name = strtok(var_name, " \t\r\n\a");
		var_value = strtok(var_value, "\n");

		if (var_name && var_value) {
			// Check if the value is inside parentheses
			if (var_value[0] == '(' && var_value[strlen(var_value) - 1] == ')') {
				var_value[strlen(var_value) - 1] = '\0';
				var_value++;
			}

			// Split the value into a list of strings
			char **values = parse_line(var_value);
			int length = 0;
			while (values[length] != NULL) {
				length++;
			}
			set_var(var_name, values, length);

			free(values);
			return status;
		}
	}

	// Extract grouped commands ({ ... } > file)
	char *grouped_commands = extract_commands(line);
	if (grouped_commands) {
		char *redirect = strchr(line, '>');
		if (redirect) {
			char *filename = strtok(redirect + 1, " \t\r\n\a");
			int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd == -1) {
				perror("open");
				free(grouped_commands);
				return status;
			}
			status = execute_with_redirection(grouped_commands, fd);
			close(fd);
		} else {
			status = execute_grouped_commands(grouped_commands);
		}
		free(grouped_commands);
		return status;
	}

	// Split and execute commands with operators (&&, ||)
	commands = split_operators(line);
	for (int i = 0; commands[i] != NULL; i++) {
        // Separate command form args
        char *command = strtok(commands[i], " ");
        if (!command)
            continue;

        char *arguments = strtok(NULL, "\0");
        char **parsed_command;
		// Substitute variables in the command
        if (arguments) {
            parsed_command = parse_concatenation(arguments);
        } else {
            parsed_command = malloc(2 * sizeof(char *));
            parsed_command[0] = strdup(command);
            parsed_command[1] = NULL;
        }

        if (parsed_command) {
            // Combined the whole thing back
            int arg_count = count_args(parsed_command);
            char **final_command = malloc((arg_count + 2) * sizeof(char *));
            if (!final_command) {
                perror("malloc");
                return 1;
            }
            final_command[0] = strdup(command);
            for (int j = 0; j < arg_count; j++) {
                    final_command[j + 1] = parsed_command[j];
            }
            final_command[arg_count + 1] = NULL;

            status = execute_line(final_command);
            free(final_command[0]);
            for (int i = 1; final_command[i] != NULL; i++) {
                free(final_command[i]);
            }
            free(final_command);
            free(parsed_command);

            if ((status == 0 && strstr(commands[i], "&&")) ||
                    (status != 0 && strstr(commands[i], "||")))
                break;
        }
	}
	free(commands);
	return status;
}

int count_args(char **args)
{
    int count = 0;
    while (args[count] != NULL) {
        count++;
    }
    return count;
}

void shell_loop()
{
	char *line = NULL;
	size_t len = 0;
	int status = 1;

	do {
		printf("nrc$ ");
		if (getline(&line, &len, stdin) == -1) {
			perror("getline");
			exit(EXIT_FAILURE);
		}

		process_command(line);

	} while (status);

	free(line);
}

int execute_line(char **args)
{
    if (args[0] == NULL)
        return 1;

    for (int i = 0; i < num_builtins(); i++) {
            if (strcmp(args[0], builtin_str[i]) == 0)
                    return (*builtin_func[i])(args);
    }

    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
            if (execvp(args[0], args) == -1)
                    perror("nrc");

            exit(EXIT_FAILURE);
    } else if (pid < 0) {
            perror("nrc");
    } else {
                do {
                        waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int execute_grouped_commands(char *commands)
{
	char *cmd;
	int status = 1;

	cmd = strtok(commands, ";");
	while (cmd != NULL) {
		char **args = parse_line(cmd);
		if (args[0] != NULL) {
			status = execute_command(args);
		}
		free(args);
		cmd = strtok(NULL, ";");
	}

	return status;
}

int execute_with_redirection(char *commands, int fd)
{
	pid_t pid;
	int status;

	if ((pid = fork()) == -1) {
		perror("fork");
		return -1;
	} else if (pid == 0) {
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		execute_grouped_commands(commands);
		exit(EXIT_SUCCESS);
	} else {
		waitpid(pid, &status, 0);
	}

	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

int execute_piped_commands(char **commands)
{
	int pipefd[2];
	pid_t pid;
	int fd_in = 0;
	int i = 0;
	int status = 0;

	while (commands[i] != NULL) {
		if (pipe(pipefd) == -1) {
			perror("pipe");
			return -1;
		}

		if ((pid = fork()) == -1) {
			perror("fork");
			return -1;
		} else if (pid == 0) {
			dup2(fd_in, 0);
			if (commands[i + 1] != NULL)
				dup2(pipefd[1], 1);

			close(pipefd[0]);
			char **args = parse_line(commands[i]);
			if (execvp(args[0], args) == -1) {
				perror("execvp");
				exit(EXIT_FAILURE);
			}
		} else {
			wait(&status);
			if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
				return -1;

			close(pipefd[1]);
			fd_in = pipefd[0];
			i++;
		}
	}

	return status;
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
			exit(EXIT_FAILURE);
		}
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
