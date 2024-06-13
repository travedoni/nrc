#ifndef SHELL_H
#define SHELL_H

int process_command(char *line);
void shell_loop();
int execute_command(char **args);
int execute_line(char *line);
int execute_grouped_commands(char *commands);
int execute_with_redirection(char *commands, int fd);
int execute_piped_commands(char **commands);

#endif
