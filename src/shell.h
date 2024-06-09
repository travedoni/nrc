#ifndef SHELL_H
#define SHELL_H

void shell_loop();
int execute_command(char **args);
void execute_piped_commands(char **commands);

#endif
