#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef struct Var {
	char *name;
	char *value;
	struct Var *next;
} Var;

Var *var_list = NULL;

void set_var(char *name, char *value)
{
	Var *var = var_list;
	while (var != NULL) {
		if (strcmp(var->name, name) == 0) {
			free(var->value);
			var->value = strdup(value);
			return;
		}
		var = var->next;
	}
	var = malloc(sizeof(Var));
	var->name = strdup(name);
	var->value = strdup(value);
	var->next = var_list;
	var_list = var;
}

char *get_var(char *name) 
{
	Var *var = var_list;
	while (var != NULL) {
		if (strcmp(var->name, name) == 0) 
			return var->value;

		var = var->next;
	}
	
	return NULL;
}
	
