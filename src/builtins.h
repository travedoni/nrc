#ifndef BUILTINS_H
#define BUILTINS_H

int shell_set(char **args);
int shell_get(char **args);
int shell_exit(char **args);
int shell_cd(char **args);
int shell_echo(char **args);
int shell_lc(char **args);
int shell_cat(char **args);
int shell_touch(char **args);
int shell_mkdir(char **args);
int shell_pwd(char **args);
int num_builtins();

extern char *builtin_str[];
extern int (*builtin_func[]) (char **);

#endif

