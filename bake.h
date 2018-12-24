#ifndef BAKE_H
#define BAKE_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

typedef struct s_map
{
    char *key;
    char *value;
} t_map;

typedef struct s_param
{
    int error;
    int change_dir;
    char *dir;
    int change_file;
    char *file;
    int ignore_error;
    int print_only;
    int print_bake;
    int silent;

} t_param;

typedef struct s_act
{
    char *key;
    char **targets;
    char **actions;
    int num_targ;
    int num_act;
} t_act;

typedef struct s_file
{
    int size;       // number of char in file
    int index;      // current position in string
    int var_count;  // count of variables
    int act_count;  // count of activities
    int num_line;   // number of line if file
    char *str;      // source string from file
    char **substrs; // array of strings from file
    char *program;
    t_map *variables;
    t_act *activities;
    t_param param;
} t_file;

char *readfile(char *filename);

#endif
