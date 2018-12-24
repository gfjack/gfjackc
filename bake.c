#include "bake.h"

char *readfile(char *filename)
{
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");

    if (handler)
    {
        fseek(handler , 0, SEEK_END);
        string_size = ftell(handler);
        rewind(handler);
        buffer = (char *)malloc(sizeof(char) * (string_size + 1));
        read_size = fread(buffer, sizeof(char), string_size, handler);
        buffer[string_size] = '\0';

        if (string_size != read_size)
        {
            free(buffer);
            buffer = NULL;
        }

        fclose(handler);
    }

    return buffer;
}

int get_var(char *str)
{
    int flag;
    flag = 0;
    while (*str != '\0')
    {
        if (*str == '#')
            return (3); // Comment
        else if (*str == '=')
            return (1); // Variable
        else if (*str == ':')
            return (2); // Activity
        str++;
    }
    return (flag);
}

int str_continue(char *str)
{
    while (*str != '\0')
    {
        if (*str == '\\')
            return (1); // Need to concatination with next line
        str++;
    }
    return (0);
}

void skip_whitespaces(char *str, int *i)
{
    int len;
    len = strlen(str);
    while (*i < len && (str[*i] == ' ' || str[*i] == '\t'))
        (*i)++;
}

char *rm_whitespaces(char *str)
{
    int i = 0, j;
    int len = strlen(str);
    char *s = strdup(str);
    skip_whitespaces(str, &i);
    j = 0;
    while (str[j] != '\0')
    {
        if (str[j] == '\\')
            str[j] = ' ';
        j++;
    }
    j = 0;
    while (i < len)
    {
        if (((str[i] == ' ') && (str[i + 1] == ' ')) || ((str[i] == ' ') && (str[i + 1] == '\t')) || ((str[i] == '\t') && (str[i + 1] == ' ')) || ((str[i] == '\t') && (str[i + 1] == '\t')))
        {
            i++;
            continue;
        }
        if (str[i] == '\t')
            s[j] = ' ';
        else
            s[j] = str[i];
        i++;
        j++;
    }
    s[j] = '\0';
    j--;
    while (j > 0 && (s[j] == ' ' || s[j] == '\t'))
    {
        s[j] = '\0';
        j--;
    }
    return (s);
}

char *concat(char *s1, char *s2, char *div)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 2);
    strcpy(result, s1);
    strcat(result, div);
    strcat(result, s2);
    return result;
}

int is_empty_str(char *str)
{
    str = rm_whitespaces(str);
    while (*str != '\0')
    {
        if (*str > 32 && *str < 127)
            return (0);
        str++;
    }
    return (1);
}

char **strsplit(const char *str, const char *delim, size_t *numtokens)
{
    char *s = strdup(str);
    size_t tokens_alloc = 1;
    size_t tokens_used = 0;
    char **tokens = calloc(tokens_alloc, sizeof(char *));
    char *token, *strtok_ctx;
    for (token = strtok_r(s, delim, &strtok_ctx);
         token != NULL;
         token = strtok_r(NULL, delim, &strtok_ctx))
    {
        // check if we need to allocate more space for tokens
        if (tokens_used == tokens_alloc)
        {
            tokens_alloc *= 2;
            tokens = realloc(tokens, tokens_alloc * sizeof(char *));
        }
        tokens[tokens_used++] = strdup(token);
    }
    // cleanup
    if (tokens_used == 0)
    {
        free(tokens);
        tokens = NULL;
    }
    else
    {
        tokens = realloc(tokens, tokens_used * sizeof(char *));
    }
    *numtokens = tokens_used;
    free(s);
    return tokens;
}

void add_commands(t_file *src, int line, int act)
{
    int i, j, k;
    int num = 0;
    int len = strlen(src->substrs[line]);
    char *tmpstr = (char *)malloc(sizeof(char) * len);
    memset(tmpstr, 0, sizeof(char) * len);

    src->activities[act].key = (char *)malloc(sizeof(char) * len);
    memset(src->activities[act].key, 0, sizeof(char) * len);

    i = 0;
    while (i < len && src->substrs[line][i] != ':')
    {
        src->activities[act].key[i] = src->substrs[line][i];
        i++;
    }
    src->activities[act].key[i] = '\0';
    src->activities[act].key = rm_whitespaces(src->activities[act].key);
    if (src->substrs[line][i] == ':')
        i++;
    j = 0;
    while (src->substrs[line][i] != '\0')
    {
        tmpstr[j] = src->substrs[line][i];
        i++;
        j++;
    }
    tmpstr[j] = '\0';
    if (str_continue(src->substrs[line]) && src->substrs[line + 1])
    {
        do
        {
            line++;
            if (!is_empty_str(src->substrs[line]))
                tmpstr = concat(tmpstr, src->substrs[line], " ");
        } while (str_continue(src->substrs[line]) && src->substrs[line + 1]);
    }
    tmpstr = rm_whitespaces(tmpstr);

    if (!is_empty_str(tmpstr))
    {
        src->activities[act].targets = strsplit(tmpstr, " ", (size_t *)&src->activities[act].num_targ);
    }
    else
        src->activities[act].num_targ = 0;

    line++;
    j = line;
    num = 0;
    while (src->substrs[j])
    {
        k = get_var(src->substrs[j]);
        if ((k == 1) || (k == 2))
            break;
        if (k == 0 && !is_empty_str(src->substrs[j]))
            num++;
        j++;
    }

    src->activities[act].num_act = num;
    if (num > 0)
    {
        src->activities[act].actions = (char **)malloc(sizeof(char *) * num);
        num = 0;
        while (src->substrs[line])
        {
            k = get_var(src->substrs[line]);
            if ((k == 1) || (k == 2))
                break;
            if (k == 0 && !is_empty_str(src->substrs[line]))
            {
                src->activities[act].actions[num] = strdup(rm_whitespaces(src->substrs[line]));
                num++;
            }
            line++;
        }
    }
    free(tmpstr);
}

void fill_map(t_file *src)
{
    int var;        // flag, result of get_var();
    int value_size; // number of characters in value;
    int i;          // counter of line;
    int j;          // counter of character in line;
    int k;          // temp counter;
    int key_i;      // index in string (key of variables);
    int val_i;      // index in string (value of variables or commands);
    int count_var;  // counter of variables;
    int count_act;  // counter of activities;

    i = 0;
    count_var = -1;
    count_act = -1;
    src->variables = malloc(sizeof(t_map) * src->var_count);
    src->activities = malloc(sizeof(t_act) * src->act_count);
    while (i < src->num_line)
    {
        j = 0;
        key_i = 0;
        var = get_var(src->substrs[i]);
        // Variables in line:
        if (var == 1)
        {
            count_var++;
            val_i = 0;
            value_size = strlen(src->substrs[i]) + 10;
            src->variables[count_var].key = malloc(sizeof(char) * strlen(src->substrs[i]));
            memset(src->variables[count_var].key, 0, sizeof(char) * strlen(src->substrs[i]));
            k = i;
            if (str_continue(src->substrs[i]))
            {
                while (str_continue(src->substrs[k]) && k < src->num_line)
                {
                    k++;
                    value_size += strlen(src->substrs[k]);
                }
            }
            src->variables[count_var].value = malloc(sizeof(char) * value_size);
            memset(src->variables[count_var].value, 0, sizeof(char) * value_size);

            while (src->substrs[i][j] != '=')
            {
                src->variables[count_var].key[key_i] = src->substrs[i][j];
                j++;
                key_i++;
            }

            src->variables[count_var].key[key_i] = '\0';
            src->variables[count_var].key = rm_whitespaces(src->variables[count_var].key);
            if (src->substrs[i][j] == '=')
                j++;
            while (src->substrs[i][j] != '\0' && src->substrs[i][j] != '\\')
            {
                src->variables[count_var].value[val_i] = src->substrs[i][j];
                j++;
                val_i++;
            }
            src->variables[count_var].value[val_i] = '\0';
            if (str_continue(src->substrs[i]) && src->substrs[i + 1])
            {
                do
                {
                    i++;
                    if (!is_empty_str(src->substrs[i]))
                        src->variables[count_var].value = concat(src->variables[count_var].value, src->substrs[i], " ");
                } while (str_continue(src->substrs[i]) && src->substrs[i + 1]);
            }
            src->variables[count_var].value = rm_whitespaces(src->variables[count_var].value);
        }
        // Activities in line:
        else if (var == 2)
        {
            count_act++;
            add_commands(src, i, count_act);
        }
        i++;
    }
}

int find_var(char *str, int i)
{
    while (str[i] != '\0')
    {
        if (str[i] == '$' && str[i + 1] == '(')
            return (i);
        i++;
    }
    return (999);
}

char *find_and_insert(char *str, char *sub_str, char *sub_str_rep)
{
    const int BUFFER_SIZE = 256;
    char *buffer = NULL;
    if (str && sub_str && sub_str_rep)
    {
        if (strlen(str) < strlen(str) - strlen(sub_str) + strlen(sub_str_rep))
        {
            buffer = (char *)malloc(BUFFER_SIZE);
            memset(buffer, 0, strlen(str));
            strncpy(buffer, str, strlen(str) - strlen(strstr(str, sub_str)));
            strcat(buffer, sub_str_rep);
            strcat(buffer, strstr(str, sub_str) + strlen(sub_str));
            strcpy(str, buffer);
            free(buffer);
        }
    }
    return str;
}

void add_value(t_file *src, char **res, char *str)
{
    char **tmp;
    int num_tmp;
    int j;

    tmp = strsplit(str, ")", (size_t *)&num_tmp);
    j = 0;
    while (src->variables[j].key && (strcmp(src->variables[j].key, tmp[0]) != 0))
        j++;
    if (strcmp(src->variables[j].key, tmp[0]) == 0)
    {
        *res = concat(*res, src->variables[j].value, "");
    }
    if (num_tmp > 1)
    {
        *res = concat(*res, tmp[1], "");
    }
    free(tmp);
}

char *replace_var(t_file *src, char *str)
{
    int i;
    int num_parts;
    char **parts;
    char *res;

    res = malloc(10);
    res[0] = '\0';
    parts = strsplit(str, "$(", (size_t *)&num_parts);
    i = 0;
    if (str[0] != '$')
    {
        res = concat(res, parts[i], "");
        i++;
    }

    if (num_parts > 1)

        while (i < num_parts)
        {
            add_value(src, &res, parts[i]);
            i++;
        }
    free(parts);
    return (res);
}

int has_dog(char *str)
{
    while (*str != '\0')
    {
        if (*str == '@')
            return (1);
        str++;
    }
    return (0);
}

void rm_dog(char *str)
{
    while (*str != '\0')
    {
        if (*str == '@')
            *str = ' ';
        str++;
    }
}

void call_activ(t_file *src, char *str)
{
    char *tmp;
    int i, j, k;
    i = 0;
    while (i < src->act_count)
    {
        if (strcmp(src->activities[i].key, str) == 0)
            break;
        i++;
    }
    if (strcmp(src->activities[i].key, str) == 0)
    {
        j = 0;
        while (j < src->activities[i].num_targ)
        {
            k = 0;
            while (k < src->act_count)
            {
                if (strcmp(src->activities[i].targets[j], src->activities[k].key) == 0)
                    break;
                k++;
            }
            if (strcmp(src->activities[i].targets[j], src->activities[k].key) == 0)
            {
                call_activ(src, src->activities[k].key);
            }
            else if (!src->param.silent)
                printf("bake: Nothing to be done for '%s'\n", src->activities[j].key);
            j++;
        }
        j = 0;
        while (j < src->activities[i].num_act)
        {
            tmp = strdup(replace_var(src, src->activities[i].actions[j]));
            if (has_dog(tmp))
            {
                rm_dog(tmp);
            }
            else if (!src->param.silent)
                printf("%s\n", rm_whitespaces(tmp));

            system(tmp);
            j++;
        }
    }
    else if (!src->param.silent)
        printf("bake: Nothing to be done for '%s'\n", str);
}

void bake(t_file *src)
{

    int i;
    int j;
    DIR *dirptr;
    char *file;
    file = strdup("");

    if (src->param.change_dir)
    {
        if ((dirptr = opendir(src->param.dir)) != NULL)
        {
            closedir(dirptr);
            chdir(src->param.dir);
        }
    }
    if (src->param.change_file && !access(src->param.file, 0))
        src->param.file = concat(file, src->param.file, "");
    else if (!access("Bakefile", 0))
        src->param.file = concat(file, "Bakefile", "");
    else if (!access("bakefile", 0))
        src->param.file = concat(file, "bakefile", "");
    else
        src->param.error = 1;
    if (!src->param.error)
    {
        src->str = strdup(readfile(src->param.file));
        src->size = strlen(src->str);
        src->num_line = 0;
        src->var_count = 0;
        src->act_count = 0;

        src->substrs = strsplit(src->str, "\n", (size_t *)&(src->num_line));
        i = 0;
        while (src->substrs[i])
        {
            switch (get_var(src->substrs[i]))
            {
            case 1:
                src->var_count += 1;
                break;
            case 2:
                src->act_count += 1;
                break;
            }
            i++;
        }
        fill_map(src);

        if (src->param.print_bake)
        {
            printf("The specification file '%s' contains:\n", src->param.file);
            printf("Variables:\n");
            for (i = 0; i < src->var_count; i++)
                printf("%d: %s = %s\n", i, replace_var(src, src->variables[i].key),
                       replace_var(src, src->variables[i].value));
            printf("\n");
            printf("Actions:\n");
            for (i = 0; i < src->act_count; i++)
            {
                j = 0;
                if (src->activities[i].num_targ > 0)
                    printf("%d: %s: Targets:\n", i, replace_var(src, src->activities[i].key));
                while (j < src->activities[i].num_targ)
                {
                    printf("   %s\n", replace_var(src, src->activities[i].targets[j]));
                    j++;
                }
                j = 0;
                if (src->activities[i].num_act > 0)
                    printf("%d: %s: Actions: \n", i, replace_var(src, src->activities[i].key));
                while (j < src->activities[i].num_act)
                {
                    printf("   %s\n", replace_var(src, src->activities[i].actions[j]));
                    j++;
                }
            }
        }
    }
}

int main(int ac, char **av)
{
    t_file *src;
    char *prog;
    int i;

    src = malloc(sizeof(t_file));
    src->param.change_dir = 0;
    src->param.change_file = 0;
    src->param.ignore_error = 0;
    src->param.print_only = 0;
    src->param.print_bake = 0;
    src->param.silent = 0;

    for (i = 1; i < ac; i++)
    {
        if (av[i][0] == '-' && strlen(av[i]) == 2)
        {
            if (av[i][1] == 'C' && (i + 1) < ac)
            {
                src->param.change_dir = 1;
                src->param.dir = strdup(av[i + 1]);
            }
            if (av[i][1] == 'f' && av[i + 1])
            {
                src->param.change_file = 1;
                src->param.file = strdup(av[i + 1]);
            }
            if (av[i][1] == 'i')
                src->param.ignore_error = 1;
            if (av[i][1] == 'n')
                src->param.print_only = 1;
            if (av[i][1] == 'p')
                src->param.print_bake = 1;
            if (av[i][1] == 's')
                src->param.silent = 1;
        }
    }
    src->program = strdup(av[0]);

    if (!src->param.error)
    {
        bake(src);
        if (ac == 1)
        {
            i = 0;
            (void)prog;
            while ((i < src->var_count) && (strcmp(src->variables[i].key, "NAME") != 0))
                i++;
            if (strcmp(src->variables[i].key, "NAME") == 0)
            {
                prog = strdup(src->variables[i].value);
            }
            else
                prog = strdup(av[0]);
            if (!access(prog, 0))
                printf("bake: *** No rule to make target '%s'.  Stop.\n", prog);
            else
                call_activ(src, "all");
        }
        else
        {
            i = 1;
            while (i < ac)
            {
                if (av[i][0] == '-')
                {
                    if ((av[i][1] == 'f' || av[i][1] == 'C')  && strlen(av[i]) == 2 && ((i + 1) < ac))
                        i++;
                }
                else
                    call_activ(src, av[i]);
                i++;
            }
        }
    }
    else if (!src->param.silent)
    {
        printf("bake: *** No targets specified and no bakefile found.  Stop.\n");
    }
    return (0);
}
