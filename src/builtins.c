#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include "builtins.h"
#include "variables.h"

char *builtin_str[] = {
	"set",
	"get",
	"exit",
	"cd",
	"echo",
	"lc",
	"cat",
	"touch",
	"mkdir",
	"pwd"
};

int (*builtin_func[]) (char **) = {
	&shell_set,
	&shell_get,
	&shell_exit,
	&shell_cd,
	&shell_echo,
	&shell_lc,
	&shell_cat,
	&shell_touch,
	&shell_mkdir,
	&shell_pwd
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


int shell_cat(char **args)
{
	int fd;
	char buffer[1024];
	ssize_t bytes_read;

	if (args[1] == NULL) {
		fprintf(stderr, "nrc: expected argument to \"cat\"\n");
		return 1;
	}

	for (int i = 1; args[i] != NULL; i++) {
		fd = open(args[i], O_RDONLY);
		if (fd == -1) {
			perror("nrc");
			continue;
		}

		while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
			write(STDOUT_FILENO, buffer, bytes_read);
		}

		close(fd);
	}

	return 1;
}	

int shell_touch(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "nrc: expected argument to \"touch\"\n");
		return 1;
	}

	for (int i = 1; args[i] != NULL; i++) {
		int fd = open(args[i], O_WRONLY | O_CREAT, 0644);
		if (fd == -1) {
			perror("nrc");
			return 1;
		}
		close(fd);
	}

	return 1;
}


int shell_mkdir(char **args) 
{
	if (args[1] == NULL) {
		fprintf(stderr, "nrc: expected argument to \"mkdir\"\n");
		return 1;
	}

	for (int i = 1; args[i] != NULL; i++) {
		if (mkdir(args[i], 0755) != 0) {
			perror("nrc");
			return 1;
		}
	}

	return 1;
}

int shell_pwd(char **args)
{
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("%s\n", cwd);
		return 1;
	} else {
		perror("nrc");
		return 1;
	}
}
