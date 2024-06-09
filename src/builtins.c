#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "utils.h"

char *builtin_str[] = {
	"set",
	"get",
	"exit"
};

int (*builtin_func[]) (char **) = {
	&shell_set,
	&shell_get,
	&shell_exit
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


