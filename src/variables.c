#include <stdlib.h>
#include <string.h>
#include "variables.h"

#define MAX_VAR 100
#define MAX_VAR_LENGTH 100

static Variable variables[MAX_VAR];
static int num_vars = 0;

Variable *find_var(const char *name)
{
	for (int i = 0; i < num_vars; i++) {
		if (strcmp(variables[i].name, name) == 0)
			return &variables[i];
	}

	return NULL;
}

void set_var(const char *name, char **value, int length)
{ 
	Variable *var = find_var(name);
	if (var) {
		free(var->value);
	} else {
		var = &variables[num_vars++];
		var->name = strdup(name);
	}
	var->value = malloc(length * sizeof(char *));
	for (int i = 0; i < length; i++) {
		var->value[i] = strdup(value[i]);
	}
	var->length = length;
}	

char **get_var(const char *name)
{
	Variable *var = find_var(name);
	if (var)
		return var->value;
	
	return NULL;
}

int get_var_length(const char *name)
{
	Variable *var = find_var(name);
	if (var) 
		return var->length;

	return 0;
}

char *concatenate_var(const char *name)
{
	Variable *var = find_var(name);
	if (var) {
		int total_length = 0;
		for (int i = 0; i < var->length; i++) {
			total_length += strlen(var->value[i]) + 1;
		}
		char *result = malloc(total_length);
		result[0] = '\0';
		for (int i = 0; i < var->length; i++) {
			strcat(result, var->value[i]);
			if (i < var->length - 1) 
				strcat(result, " ");

		}
		return result;
	}
	return NULL;
}

