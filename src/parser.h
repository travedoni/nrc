#ifndef PARSER_H
#define PARSER_H

char **parse_line(char *line);
char **split_operators(char *line);
char **split_pipes(char *line);

#endif
