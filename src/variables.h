#ifndef VARIABLES_H
#define VARIABLES_H

#include <string.h>

typedef struct {
	char *name;
	char **value;
	int length;
} Variable;

Variable *find_var(const char *name);
void set_var(const char *name, char **value, int length);
char **get_var(const char *name);
int get_var_length(const char *name); 
char *concatenate_var(const char *name);
char *get_subscript(char *arg);

#endif
