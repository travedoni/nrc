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

		if (strchr(line, '=')) {
			char *var_name = strtok(line, "=");
			char *var_value = strtok(NULL, "\n");
			if (var_name && var_value) {
				char **values = parse_line(var_value);
				int length = 0;
				while (values[length] != NULL) {
					length++;
				}
				set_var(var_name, values, length);
				free(values);
				continue;
			}
		}

		char *grouped_commands = extract_commands(line);
		if (grouped_commands) {
			char *redirect = strchr(line, '>');
			if (redirect) {
				char *filename = strtok(redirect + 1, " \t\r\n\a");
				int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (fd == -1) {
					perror("open");
					free(grouped_commands);
					continue;
				}
				status = execute_with_redirection(grouped_commands, fd);
				close(fd);
			} else {
				status = execute_grouped_commands(grouped_commands);
			}
			free(grouped_commands);
			continue;
		}

		commands = split_operators(line);
		for (int i = 0; commands[i] != NULL; i++) {
			char *substituted_command = substitute_vars(commands[i]);
			status = execute_line(substituted_command);
			free(substituted_command);
			if (status == 0) 
				break;
		}
		free(commands);
	} while (status);

	free(line);
}

int execute_line(char *line)
{
	char **commands = split_operators(line);
	int status = 1;

	for (int i = 0; commands[i] != NULL; i++) {
		char **pipe_segments = split_pipes(commands[i]);
		if (pipe_segments[1] == NULL) {
			char **args = parse_line(pipe_segments[0]);
			if (args[0] != NULL) 
				status = execute_command(args);

			free(args);
		} else {
			status = execute_piped_commands(pipe_segments);
		}
		free(pipe_segments);

		// Handle && and || operators
		if (commands[i + 1] != NULL) {
			if (strstr(commands[i], "&&") != NULL && status != 1) {
				break;
			} else if (strstr(commands[i], "||") != NULL && status != 1) {
				break;
			}
		}
	}
	free(commands);

	return status;
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
