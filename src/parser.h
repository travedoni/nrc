#ifndef PARSER_H
#define PARSER_H

char *substitute_vars(char *arg); 
char **parse_line(char *line);
char **expand_patterns(char **args);
char **split_commands(char *line);
char **split_operators(char *line);
char **split_pipes(char *line);
char *extract_commands(char *line);
char *get_subscript(char *arg);
char *substitute_vars(char *arg);

#endif
