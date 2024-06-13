#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "variables.h"

void execute_script(const char *script_name) 
{
	FILE *script = fopen(script_name, "r");
	if (script == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, script) != -1) {
		// Process each line as a command
		process_command(line);
	}

	fclose(script);
	free(line);
}

int main(int argc, char **argv) 
{
	if (argc > 0) {
		char *script_name = argv[0];
		set_var("0", &script_name, 1);
	}

	if (argc > 1) {
		set_var("*", &argv[1], argc - 1);

		for (int i = 1; i < argc; i++) {
			char arg_name[10];
			snprintf(arg_name, sizeof(arg_name), "%d", i);
			set_var(arg_name, &argv[i], 1);
		}
	}

	// If the first argument is a script starting with "./"
	if (argc > 1 && strncmp(argv[1], "./", 2) == 0) {
		execute_script(argv[1]);
	} else {
		shell_loop();
	}

	return 0;
}
