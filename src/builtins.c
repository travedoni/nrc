#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include "builtins.h"
#include "variables.h"
#include "parser.h"

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

void list_directory(const char *path, int all, int long_format, int recursive) 
{
	struct stat path_stat;
	if (stat(path, &path_stat) != 0) {
		perror("stat");
		return;
	}
	
	if (S_ISDIR(path_stat.st_mode)) {
		// Handle directory
		DIR *dir = opendir(path);
		if (!dir) {
			perror("opendir");
			return;
		}

		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL) {
			if (!all && entry->d_name[0] == '.') {
				continue;
			}

			print_file_info(path, entry, long_format);
		}

		if (!long_format) 
			printf("\n");

		closedir(dir);

		if (recursive) {
			dir = opendir(path);
			if (!dir) {
				perror("opendir");
				return;
			}
			while ((entry = readdir(dir)) != NULL) {
				if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
						strcmp(entry->d_name, "..") != 0) {
					char next_path[PATH_MAX];
					snprintf(next_path, sizeof(next_path), "%s/%s", path, entry->d_name);
					printf("\n%s:\n", next_path);
					list_directory(next_path, all, long_format, recursive);
				}
			}
			closedir(dir);
		}
	} else {
		// Handle file
		print_file_info(".", NULL, long_format);
	}
}	

void print_file_info(const char *path, const struct dirent *entry, int long_format) 
{
	if (long_format) {
		struct stat file_stat;
		char full_path[PATH_MAX];
		snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
		if (stat(full_path, &file_stat) == -1) {
			perror("stat");
			return;
		}

		// File type and permissions
		printf("%c", S_ISDIR(file_stat.st_mode) ? 'd' : '-');
		printf("%c", file_stat.st_mode & S_IRUSR ? 'r' : '-');
		printf("%c", file_stat.st_mode & S_IWUSR ? 'w' : '-');
		printf("%c", file_stat.st_mode & S_IXUSR ? 'x' : '-');
		printf("%c", file_stat.st_mode & S_IRGRP ? 'r' : '-');
		printf("%c", file_stat.st_mode & S_IWGRP ? 'w' : '-');
		printf("%c", file_stat.st_mode & S_IXGRP ? 'x' : '-');
		printf("%c", file_stat.st_mode & S_IROTH ? 'r' : '-');
		printf("%c", file_stat.st_mode & S_IWOTH ? 'w' : '-');
		printf("%c", file_stat.st_mode & S_IXOTH ? 'x' : '-');
		printf(" ");

		// Number of hard links
		printf("%hu ", file_stat.st_nlink);

		// Owner and group
		struct passwd *pw = getpwuid(file_stat.st_uid);
		struct group *gr = getgrgid(file_stat.st_gid);
		printf("%s %s ", pw->pw_name, gr->gr_name);

		// Size
		printf("%lld ", file_stat.st_size);

		// Last modified time
		char time_str[20];
		strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file_stat.st_mtime));
		printf("%s ", time_str);

		// File name
		printf("%s\n", entry->d_name);
	} else {
		printf("%s  ", entry->d_name);
	}
}

int shell_set(char **args) 
{
	if (args[1] == NULL || args[2] == NULL) {
		fprintf(stderr, "nrc: expected variable name and value\n");
		return 1;
	}
	set_var(args[1], &args[2], 1);
	return 1;
}

int shell_get(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "nrc: expected variable name\n");
		return 1;
	}
	char **value = get_var(args[1]);
	if (value != NULL) {
		printf("%s\n", *value);
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
	int all = 0;
	int long_format = 0;
	int recursive = 0;
	char *path = ".";

	// Parse flags and arguments
	char **expanded_args = expand_patterns(args);

	for (int i = 1; expanded_args[i] != NULL; i++) {
		if (expanded_args[i][0] == '-') {
			for (char *c = expanded_args[i] + 1; *c != '\0'; c++) {
				switch (*c) {
					case 'a':
						all = 1;
						break;
					case 'l':
						long_format = 1;
						break;
					case 'R':
						recursive = 1;
						break;
					default:
						fprintf(stderr, "nrc: lc: invalid option -- '%c'\n", *c);
						return 1;
				}
			}
		} else {
			path = expanded_args[i];
		}
	}

	list_directory(path, all, long_format, recursive);

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
	int logical = 1;
	char cwd[PATH_MAX];
	char *env_pwd = getenv("PWD");

	// Parse flag
	for (int i = 1; args[i] != NULL; i++) {
		if (strcmp(args[i], "-P") == 0) {
			logical = 0;
		} else if (strcmp(args[i], "-L") == 0) {
			logical = 1;
		} else {
			fprintf(stderr, "nrc: pwd: invalid option --'%s'\n", args[i]);
			return 1;
		}
	}

	if (logical && env_pwd != NULL) {
		printf("%s\n", env_pwd);
	} else {
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			printf("%s\n", cwd);
			return 1;
		} else {
			perror("nrc");
			return 1;
		}
	}

	return 1;
}
