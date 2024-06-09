#ifndef SHELL_H
#define SHELL_H

void shell_loop();
char **parse_line(char *line);
int execute_command(char **args);

#endif
