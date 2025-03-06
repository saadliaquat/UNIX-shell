/**
 * @file main.c
 * @brief This is the main file for the shell. It contains the main function and the loop that runs the shell.
 * @version 0.1
 * @date 2023-06-02
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "log.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <glob.h>

/**
 * @brief This is the main function for the shell. It contains the main loop that runs the shell.
 *
 * @return int
 */

void cleararray(char command_args[][300])
{
    char *end = "\0";
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            command_args[i][j] = end[0];
        }
    }
}

int add_in_alias(char alias[][200], char alias_command[][200], int no_of_alias, char command_args[][300])
{
    int found_alias = 0;
    for (int i = 0; i < no_of_alias; i++)
    {
        if (strcmp(alias[i], command_args[1]) == 0)
        {
            found_alias = 1;
            for (long unsigned int y = 0; y < 200; y++)
            {
                char *end = "\0";
                alias_command[i][y] = end[0];
            }
            for (long unsigned int j = 0; j < strlen(command_args[2]); j++)
            {
                alias_command[i][j] = command_args[2][j];
            }
            break;
        }
    }
    if (found_alias != 1)
    {
        for (long unsigned int i = 0; i < strlen(command_args[1]); i++)
        {
            alias[no_of_alias][i] = command_args[1][i];
        }
        for (long unsigned int i = 0; i < strlen(command_args[2]); i++)
        {
            alias_command[no_of_alias][i] = command_args[2][i];
        }
        no_of_alias = no_of_alias + 1;
    }
    return no_of_alias;
}
int parser(char command_args[][300], int no_of_commands, char *input)
{
    int use_space_as_delim = 1;
    int no_of_char_in_string = 0;
    int no_of_inverted = 0;
    for (long unsigned int i = 0; i < strlen(input); i++)
    {
        int ascii = 34;
        int ascii2 = 39;
        char inverted_comma = (char)ascii;
        char single_inverted_comma = (char)ascii2;
        char *end = "\0";
        char *space = " ";
        if (input[i] == inverted_comma || input[i] == single_inverted_comma)
        {
            no_of_inverted = no_of_inverted + 1;
            if (no_of_inverted % 2 == 0)
            {
                use_space_as_delim = 1;
            }
            else
            {
                use_space_as_delim = 0;
            }
        }
        if (use_space_as_delim == 1 && input[i] == space[0])
        {
            if (input[i + 1] != end[0])
            {
                no_of_commands = no_of_commands + 1;
                no_of_char_in_string = 0;
            }
        }
        if (input[i] != inverted_comma && input[i] != single_inverted_comma)
        {
            if (use_space_as_delim == 0 && input[i] == space[0])
            {
                command_args[no_of_commands][no_of_char_in_string] = input[i];
                no_of_char_in_string = no_of_char_in_string + 1;
            }
            else if (input[i] != space[0])
            {
                command_args[no_of_commands][no_of_char_in_string] = input[i];
                no_of_char_in_string = no_of_char_in_string + 1;
            }
        }
    }
    return no_of_commands + 1;
}

void store_input(char *input_from_file[], int input_from_file_counter, char buffer[])
{
    input_from_file[input_from_file_counter] = buffer;
}

int check_pipeline(char *input)
{
    char *end = "\0";
    char *delim = "|";
    char *space = " ";
    for (long unsigned int i = 0; i < strlen(input); i++)
    {
        if (input[i + 1] != end[0])
        {
            if (input[i] == delim[0])
            {
                if (input[i - 1] == space[0] && input[i + 1] == space[0])
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int check_builtin(char command_args[][300], int no_of_commands)
{
    for (int i = 0; i < no_of_commands; i++)
    {
        if ((strcmp(command_args[i], "echo") == 0) || (strcmp(command_args[i], "cd") == 0) || (strcmp(command_args[i], "pwd") == 0) || (strcmp(command_args[i], "alias") == 0) || (strcmp(command_args[i], "unalias") == 0) || (strcmp(command_args[i], "history") == 0) || (strcmp(command_args[i], "exit") == 0))
        {
            return 1;
        }
    }
    return 0;
}

int check_valid_pipeline(char *chain_compare[100], int index_of_pipeline, int index_of_and_or_minus_one, int first_index_of_pipe)
{
    int valid_input = 0;
    int valid_output = 0;
    int count_output_new = 0;
    int count_output_append = 0;
    int count_input = 0;
    for (int i = index_of_pipeline; i < index_of_and_or_minus_one; i++)
    {
        if (strcmp(chain_compare[i], ">") == 0)
        {
            count_output_new = count_output_new + 1;
        }
        else if (strcmp(chain_compare[i], ">>") == 0)
        {
            count_output_append = count_output_append + 1;
        }
        else if (strcmp(chain_compare[i], "<") == 0)
        {
            count_input = count_input + 1;
            ;
        }
    }

    if (count_input == 1)
    {
        int index_of_sign = 0;
        for (int i = index_of_pipeline; i < index_of_and_or_minus_one; i++)
        {
            if (strcmp(chain_compare[i], "<") == 0)
            {
                index_of_sign = i;
            }
        }
        if (index_of_sign > first_index_of_pipe)
        {
            valid_input = 0;
        }
        else
        {
            valid_input = 1;
        }
    }
    else if (count_input == 0)
    {
        valid_input = 1;
    }
    else
    {
        valid_input = 0;
    }

    if (count_output_append == 1 && count_output_new == 0)
    {
        if (strcmp(chain_compare[index_of_and_or_minus_one - 2], ">>") == 0)
        {
            valid_output = 1;
        }
        else
        {
            valid_output = 0;
        }
    }
    else if (count_output_append == 0 && count_output_new == 1)
    {
        if (strcmp(chain_compare[index_of_and_or_minus_one - 2], ">") == 0)
        {
            valid_output = 1;
        }
        else
        {
            valid_output = 0;
        }
    }
    else if (count_output_append == 0 && count_output_new == 0)
    {
        valid_output = 1;
    }
    else
    {
        valid_output = 0;
    }

    if (valid_output == 1 && valid_input == 1)
    {
        return 1;
    }
    return 0;
}

int parser_for_wildcard(char command_args[][300], int no_of_commands, char *input)
{
    int use_space_as_delim = 1;
    int no_of_char_in_string = 0;
    int no_of_inverted = 0;
    cleararray(command_args);
    for (long unsigned int i = 0; i < strlen(input); i++)
    {
        int ascii = 34;
        int ascii2 = 39;
        char inverted_comma = (char)ascii;
        char single_inverted_comma = (char)ascii2;
        char *end = "\0";
        char *space = " ";

        if (input[i] == inverted_comma || input[i] == single_inverted_comma)
        {
            no_of_inverted = no_of_inverted + 1;
            if (no_of_inverted % 2 == 0)
            {
                use_space_as_delim = 1;
            }
            else
            {
                use_space_as_delim = 0;
            }
        }
        if (use_space_as_delim == 1 && input[i] == space[0])
        {
            if (input[i + 1] != end[0])
            {
                no_of_commands = no_of_commands + 1;
                no_of_char_in_string = 0;
            }
        }
        else if (use_space_as_delim == 0 && input[i] == space[0])
        {
            command_args[no_of_commands][no_of_char_in_string] = input[i];
            no_of_char_in_string = no_of_char_in_string + 1;
        }
        else if (input[i] != space[0])
        {
            command_args[no_of_commands][no_of_char_in_string] = input[i];
            no_of_char_in_string = no_of_char_in_string + 1;
        }
    }
    return no_of_commands + 1;
}

int check_wildcard(char command_chaining[][300], char command_args[][300], int no_of_commands, int previous_index_of_pipeline, int index_of_pipeline, char *input)
{
    cleararray(command_args);
    char command_chaining_quo[100][300] = {"\0"};
    cleararray(command_chaining_quo);
    int no_of_commands_quotes = 0;

    no_of_commands_quotes = parser_for_wildcard(command_chaining_quo, no_of_commands_quotes, input);

    int ascii = 34;
    int ascii2 = 39;
    char inverted_comma = (char)ascii;
    char single_inverted_comma = (char)ascii2;
    no_of_commands = 0;
    char *star = "*";
    char *question = "?";
    for (int i = previous_index_of_pipeline; i < index_of_pipeline; i++)
    {
        int wild = 0;
        for (long unsigned int j = 0; j < strlen(command_chaining[i]); j++)
        {
            if (command_chaining[i][j] == star[0])
            {
                if (command_chaining_quo[i][0] == single_inverted_comma || command_chaining_quo[i][0] == inverted_comma)
                {
                    wild = 0;
                    break;
                }
                else
                {
                    wild = 1;
                    break;
                }
            }
            else if (command_chaining[i][j] == question[0])
            {
                if (command_chaining_quo[i][0] == single_inverted_comma || command_chaining_quo[i][0] == inverted_comma)
                {
                    wild = 0;
                    break;
                }
                else
                {
                    wild = 1;
                    break;
                }
            }
        }

        if (wild == 0)
        {
            for (long unsigned int j = 0; j < strlen(command_chaining[i]); j++)
            {
                command_args[no_of_commands][j] = command_chaining[i][j];
            }
            no_of_commands = no_of_commands + 1;
        }
        else if (wild == 1)
        {
            glob_t glob_result;
            char *pattern = command_chaining[i];

            // Perform wildcard expansion
            int ret = glob(pattern, 0, NULL, &glob_result);
            if (ret != 0)
            {
                return -1;
            }

            // Iterate over the matched files
            char *temp_glob_store[100] = {NULL};
            int no_of_temp_store = 0;
            for (size_t z = 0; z < glob_result.gl_pathc; ++z)
            {
                temp_glob_store[no_of_temp_store] = glob_result.gl_pathv[z];
                no_of_temp_store = no_of_temp_store + 1;
            }
            for (int t = 0; t < no_of_temp_store; t++)
            {
                for (long unsigned int r = 0; r < strlen(temp_glob_store[t]); r++)
                {
                    command_args[no_of_commands][r] = temp_glob_store[t][r];
                }
                no_of_commands = no_of_commands + 1;
            }

            // Free the allocated memory
            globfree(&glob_result);
        }
    }
    return no_of_commands;
}

int main(int argc, char *argv[])
{
    // Configure readline to auto -complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);
    // Enable history
    using_history();

    // char* input;
    char history[100][200];
    int no_of_history = 0;
    char alias[100][200];
    int no_of_alias = 0;
    char alias_command[100][200];

    FILE *file_read_pointer;
    int read_file = 0;

    // int input_from_file = 0;

    int pipeline_exists = 0;

    int in = STDIN_FILENO;

    char chaining_command[100][300];
    cleararray(chaining_command);

    char chaining_command_with_quotes[100][300];
    cleararray(chaining_command_with_quotes);

    int index_of_sequential = 0;
    int previous_index_of_sequential = 0;

    int index_of_and_or = 0;
    int previous_index_of_and_or = 0;

    int and_bool = 0;
    int or_bool = 0;

    int index_of_pipeline = 0;
    int previous_index_of_pipeline = 0;

    int command_success = 1;
    int prev_command_success = 0;

    if (argc == 2)
    {
        read_file = 1;
        file_read_pointer = fopen(argv[1], "r");
    }
    while (1)
    {
        char *input = NULL;
        index_of_and_or = 0;
        index_of_sequential = 0;
        previous_index_of_and_or = 0;
        char file_read_buffer[MAX_STRING_LENGTH];
        in = STDIN_FILENO;
        if (read_file == 1)
        {
            if (fgets(file_read_buffer, sizeof(file_read_buffer), file_read_pointer) != NULL)
            {
                file_read_buffer[strcspn(file_read_buffer, "\n")] = 0;
                input = strdup(file_read_buffer);
            }
            else
            {
                break;
            }
        }
        else
        {
            input = readline("prompt > ");
        }
        // Check for EOF.
        if (!input)
            break;

        add_history(input);

        strcpy(history[no_of_history], input);
        no_of_history = no_of_history + 1;

        char *sequential = ";";
        int no_of_chaining_command = 0;
        int no_of_chaining_commands_with_quotes = 0;
        cleararray(chaining_command);
        no_of_chaining_command = parser(chaining_command, no_of_chaining_command, input);

        no_of_chaining_commands_with_quotes = parser_for_wildcard(chaining_command_with_quotes, no_of_chaining_commands_with_quotes, input);

        char *chain_compare[100] = {NULL};
        for (int i = 0; i < no_of_chaining_command; i++)
        {
            chain_compare[i] = chaining_command_with_quotes[i];
        }

        do
        {
            previous_index_of_sequential = index_of_sequential;
            for (int i = index_of_sequential; i < no_of_chaining_command; i++)
            {
                if (i == no_of_chaining_command - 1)
                {
                    index_of_sequential = i + 1;
                    break;
                }
                if (strcmp(chain_compare[i], sequential) == 0)
                {
                    index_of_sequential = i;
                    break;
                }
            }

            index_of_sequential = index_of_sequential + 1;
            index_of_and_or = previous_index_of_sequential;
            previous_index_of_and_or = previous_index_of_sequential;
            do // loop into sequential part;
            {
                prev_command_success = command_success;
                int prev_and = and_bool;
                int prev_or = or_bool;
                and_bool = 0;
                or_bool = 0;
                previous_index_of_and_or = index_of_and_or;
                for (int z = index_of_and_or; z < index_of_sequential - 1; z++)
                {

                    and_bool = 0;
                    or_bool = 0;
                    if (z == index_of_sequential - 2)
                    {
                        index_of_and_or = z + 2;
                        break;
                    }
                    else if (strcmp(chain_compare[z], "&&") == 0)
                    {
                        and_bool = 1;
                        index_of_and_or = z;
                        break;
                    }
                    else if (strcmp(chain_compare[z], "||") == 0)
                    {
                        or_bool = 1;
                        index_of_and_or = z;
                        break;
                    }
                }
                if (and_bool == 1 || or_bool == 1)
                {

                    index_of_and_or = index_of_and_or + 1;
                }
                if (prev_command_success == 0)
                {
                    if (prev_and == 1)
                    {
                        continue;
                    }
                }
                else if (prev_command_success == 1)
                {
                    if (prev_or == 1)
                    {
                        continue;
                    }
                }

                index_of_pipeline = previous_index_of_and_or;
                previous_index_of_pipeline = previous_index_of_and_or;
                do // loop into and or part
                {
                    int valid_pipeline = 1;
                    previous_index_of_pipeline = index_of_pipeline;
                    for (int j = index_of_pipeline; j < index_of_and_or - 1; j++)
                    {
                        if (j == index_of_and_or - 2)
                        {
                            if (pipeline_exists == 1)
                            {
                                pipeline_exists = -1;
                            }

                            index_of_pipeline = j + 1;
                            break;
                        }
                        if (strcmp(chain_compare[j], "|") == 0)
                        {
                            if (pipeline_exists == 0)
                            {
                                valid_pipeline = check_valid_pipeline(chain_compare, index_of_pipeline, index_of_and_or - 1, j);
                                if (valid_pipeline == 0)
                                {
                                    command_success = 0;
                                    break;
                                }
                            }
                            pipeline_exists = 1;
                            index_of_pipeline = j; // stores index |
                            break;
                        }
                    }
                    if (valid_pipeline == 0)
                    {
                        break;
                    }

                    int no_of_commands = index_of_pipeline - previous_index_of_pipeline;

                    char command_args[100][300];
                    cleararray(command_args);
                    int iterator = 0;
                    for (int t = previous_index_of_pipeline; t < index_of_pipeline; t++)
                    {

                        for (long unsigned int u = 0; u < strlen(chaining_command[t]); u++) // to give command_arg starting with 0 index trasferring content
                        {
                            command_args[iterator][u] = chaining_command[t][u];
                        }
                        iterator = iterator + 1;
                    }

                    index_of_pipeline = index_of_pipeline + 1;

                    no_of_commands = check_wildcard(chaining_command, command_args, no_of_commands, previous_index_of_pipeline, index_of_pipeline - 1, input);
                    if (no_of_commands == -1)
                    {
                        command_success = 0;
                        break;
                    }

                    FILE *file_input = NULL;
                    char *input_sign = "<";
                    char *filename_input;
                    int input_rejected = 0;
                    int input_from_file = 0;
                    char *commands_for_comp_input[100];
                    for (int i = 0; i < no_of_commands; i++)
                    {
                        commands_for_comp_input[i] = command_args[i];
                    }
                    for (int i = 0; i < no_of_commands; i++)
                    {
                        if (strcmp(commands_for_comp_input[i], input_sign) == 0)
                        {
                            input_from_file = 1;
                            filename_input = command_args[i + 1];
                            file_input = fopen(filename_input, "r");
                            if (file_input == NULL)
                            {
                                input_rejected = 1;
                                break;
                            }
                            no_of_commands = no_of_commands - 2;
                            int builtin_check = check_builtin(command_args, no_of_commands);
                            if (builtin_check == 1)
                            {
                                char buffer_input[MAX_STRING_LENGTH];
                                char new_input[MAX_STRING_LENGTH];
                                for (int y = 0; y < 100; y++)
                                {
                                    char *end = "\0";
                                    new_input[y] = end[0];
                                }
                                for (int q = 0; q < i; q++)
                                {
                                    strcat(new_input, command_args[q]);
                                    strcat(new_input, " ");
                                }
                                while (1)
                                {
                                    if (fgets(buffer_input, sizeof(buffer_input), file_input) != NULL)
                                    {
                                        buffer_input[strcspn(buffer_input, "\n")] = 0;
                                        strcat(new_input, buffer_input);
                                        memset(buffer_input, '\0', sizeof(buffer_input));
                                    }
                                    else
                                    {

                                        break;
                                    }
                                }

                                cleararray(command_args);
                                no_of_commands = 0;
                                no_of_commands = parser(command_args, no_of_commands, new_input);
                            }
                            break;
                        }
                    }
                    if (input_rejected == 1)
                    {
                        break;
                    }

                    char *output2 = ">>"; // checks if output into file
                    char *output = ">";
                    char *filename;
                    int output_into_file = 0;
                    FILE *file_write = NULL;
                    char *commands_for_comp[100];
                    for (int i = 0; i < no_of_commands; i++)
                    {
                        commands_for_comp[i] = command_args[i];
                    }
                    for (int i = 0; i < no_of_commands; i++)
                    {
                        if (strcmp(commands_for_comp[i], output) == 0)
                        {
                            output_into_file = 1;
                            filename = command_args[i + 1];
                            file_write = fopen(filename, "w");
                            no_of_commands = no_of_commands - 2;
                            break;
                        }
                        else if (strcmp(commands_for_comp[i], output2) == 0)
                        {
                            output_into_file = 1;
                            filename = command_args[i + 1];
                            file_write = fopen(filename, "a");
                            no_of_commands = no_of_commands - 2;
                            break;
                        }
                    }

                    if (no_of_commands - 1 == 1 && (strcmp(command_args[0], "history") == 0)) // indexes history array
                    {
                        for (int i = 0; i < no_of_history; i++)
                        {
                            int index = atoi(command_args[1]);
                            if (strcmp(history[i], history[index - 1]) == 0)
                            {
                                cleararray(command_args);
                                no_of_commands = 0;
                                no_of_commands = parser(command_args, no_of_commands, history[i]);
                                break;
                            }
                        }
                    }

                    for (int i = 0; i < no_of_alias; i++) // searches for alias stored
                    {
                        if (strcmp(alias[i], command_args[0]) == 0)
                        {
                            char temp_store[100][300];
                            cleararray(temp_store);
                            for (int p = 0; p < no_of_commands; p++)
                            {
                                for (long unsigned int l = 0; l < strlen(command_args[p]); l++)
                                {
                                    temp_store[p][l] = command_args[p][l];
                                }
                            }
                            int old_num_commands = no_of_commands;
                            cleararray(command_args);
                            no_of_commands = 0;
                            no_of_commands = parser(command_args, no_of_commands, alias_command[i]);
                            for (int p = 1; p < old_num_commands; p++)
                            {
                                for (long unsigned int l = 0; l < strlen(temp_store[p]); l++)
                                {
                                    command_args[no_of_commands][l] = temp_store[p][l];
                                }
                                no_of_commands = no_of_commands + 1;
                            }
                            break;
                        }
                    }

                    if (strcmp(command_args[0], "pwd") == 0) // pwd
                    {
                        if (no_of_commands - 1 == 0)
                        {
                            if (pipeline_exists == 1)
                            {
                                int fd[2];
                                if (pipe(fd) == -1)
                                {
                                    continue;
                                }

                                pid_t pid = fork();

                                if (pid == -1)
                                {
                                    exit(1);
                                }
                                else if (pid == 0)
                                {
                                    if (output_into_file == 1)
                                    {
                                        if (dup2(fileno(file_write), 1) == -1)
                                        {
                                            exit(1);
                                        }
                                    }
                                    if (input_from_file == 1)
                                    {
                                        if (dup2(fileno(file_input), 0) == -1)
                                        {
                                            exit(1);
                                        }
                                    }

                                    close(fd[0]);
                                    if (STDIN_FILENO != in)
                                    {
                                        if (dup2(in, STDIN_FILENO) == -1)
                                        {
                                            exit(1);
                                        }
                                        close(in);
                                    }

                                    if (STDOUT_FILENO != fd[1])
                                    {
                                        if (dup2(fd[1], STDOUT_FILENO) == -1)
                                        {
                                            exit(1);
                                        }
                                        close(fd[1]);
                                    }

                                    char current_directory[256];
                                    if (getcwd(current_directory, 256) == NULL)
                                    {
                                        exit(1);
                                        command_success = 0;
                                    }

                                    command_success = 1;
                                    printf("%s\n", getcwd(current_directory, 256));
                                    exit(0);
                                }
                                else
                                {
                                    command_success = 1;
                                    int wait_for_child;
                                    wait(&wait_for_child);
                                    close(fd[1]);
                                    in = fd[0];

                                    int exitstatus = WEXITSTATUS(wait_for_child);
                                    if (exitstatus == 1)
                                    {
                                        command_success = 0;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                char current_directory[256];
                                if (getcwd(current_directory, 256) == NULL)
                                {
                                    command_success = 0;
                                }

                                if (output_into_file == 0)
                                {
                                    command_success = 1;
                                    printf("%s\n", getcwd(current_directory, 256));
                                }
                                else if (output_into_file == 1)
                                {

                                    command_success = 1;
                                    char *path = getcwd(current_directory, 256);

                                    fprintf(file_write, "%s\n", path);
                                }
                            }
                        }
                        else
                        {
                            command_success = 0;
                        }
                    }

                    else if (strcmp(command_args[0], "exit") == 0) // exit
                    {
                        if (no_of_commands - 1 == 0)
                        {
                            command_success = 1;
                            exit(0);
                        }
                        else
                        {
                            command_success = 0;
                        }
                    }
                    else if (strcmp(command_args[0], "cd") == 0) // cd
                    {
                        if (no_of_commands - 1 == 0)
                        {
                            char *path = "/home";
                            int result = chdir(path);
                            if (result == -1)
                            {
                                command_success = 0;
                            }
                            else
                            {
                                command_success = 1;
                            }
                        }
                        else if (no_of_commands - 1 == 1)
                        {
                            int result = chdir(command_args[1]);
                            if (result == -1)
                            {
                                command_success = 0;
                            }
                            else
                            {
                                command_success = 1;
                            }
                        }
                        else if (no_of_commands - 1 > 1)
                        {
                            command_success = 0;
                        }
                    }
                    else if (strcmp(command_args[0], "history") == 0) // history
                    {
                        if (no_of_commands - 1 == 0)
                        {
                            command_success = 1;

                            if (output_into_file == 0)
                            {
                                for (int i = 0; i < no_of_history; i++)
                                {
                                    printf("%d ", i + 1);
                                    printf("%s\n", history[i]);
                                }
                            }
                            else if (output_into_file == 1)
                            {
                                for (int i = 0; i < no_of_history; i++)
                                {
                                    fprintf(file_write, "%d ", i + 1);
                                    fprintf(file_write, "%s\n", history[i]);
                                }
                            }
                        }
                        else if (no_of_commands - 1 >= 2)
                        {
                            command_success = 0;
                        }
                    }
                    else if (strcmp(command_args[0], "echo") == 0) // echo
                    {
                        command_success = 1;
                        for (int i = 1; i < no_of_commands; i++)
                        {
                            if (i == no_of_commands - 1)
                            {
                                if (pipeline_exists == 1)
                                {
                                    int fd[2];
                                    if (pipe(fd) == -1)
                                    {
                                        continue;
                                    }

                                    pid_t pid = fork();

                                    if (pid == -1)
                                    {
                                        exit(1);
                                    }
                                    else if (pid == 0)
                                    {
                                        if (output_into_file == 1)
                                        {
                                            if (dup2(fileno(file_write), 1) == -1)
                                            {
                                                exit(1);
                                            }
                                        }
                                        if (input_from_file == 1)
                                        {
                                            if (dup2(fileno(file_input), 0) == -1)
                                            {
                                                exit(1);
                                            }
                                        }

                                        close(fd[0]);
                                        if (STDIN_FILENO != in)
                                        {
                                            if (dup2(in, STDIN_FILENO) == -1)
                                            {
                                                exit(1);
                                            }
                                            close(in);
                                        }

                                        if (STDOUT_FILENO != fd[1])
                                        {
                                            if (dup2(fd[1], STDOUT_FILENO) == -1)
                                            {
                                                exit(1);
                                            }
                                            close(fd[1]);
                                        }

                                        printf("%s\n", command_args[i]);
                                        exit(0);
                                    }
                                    else
                                    {
                                        command_success = 1;
                                        int wait_for_child;
                                        wait(&wait_for_child);
                                        close(fd[1]);
                                        in = fd[0];

                                        int exitstatus = WEXITSTATUS(wait_for_child);
                                        if (exitstatus == 1)
                                        {
                                            command_success = 0;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    if (output_into_file == 0)
                                    {
                                        printf("%s\n", command_args[i]);
                                    }
                                    else if (output_into_file == 1)
                                    {
                                        fprintf(file_write, "%s\n", command_args[i]);
                                    }
                                }
                            }
                            else
                            {
                                if (pipeline_exists == 1)
                                {
                                    int fd[2];
                                    if (pipe(fd) == -1)
                                    {
                                        continue;
                                    }

                                    pid_t pid = fork();

                                    if (pid == -1)
                                    {
                                        exit(1);
                                    }
                                    else if (pid == 0)
                                    {
                                        if (output_into_file == 1)
                                        {
                                            if (dup2(fileno(file_write), 1) == -1)
                                            {
                                                exit(1);
                                            }
                                        }
                                        if (input_from_file == 1)
                                        {
                                            if (dup2(fileno(file_input), 0) == -1)
                                            {
                                                exit(1);
                                            }
                                        }

                                        close(fd[0]);
                                        if (STDIN_FILENO != in)
                                        {
                                            if (dup2(in, STDIN_FILENO) == -1)
                                            {
                                                exit(1);
                                            }
                                            close(in);
                                        }

                                        if (STDOUT_FILENO != fd[1])
                                        {
                                            if (dup2(fd[1], STDOUT_FILENO) == -1)
                                            {
                                                exit(1);
                                            }
                                            close(fd[1]);
                                        }

                                        printf("%s\n", command_args[i]);
                                        exit(0);
                                    }
                                    else
                                    {
                                        command_success = 1;
                                        int wait_for_child;
                                        wait(&wait_for_child);
                                        close(fd[1]);
                                        in = fd[0];

                                        int exitstatus = WEXITSTATUS(wait_for_child);
                                        if (exitstatus == 1)
                                        {
                                            command_success = 0;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    if (output_into_file == 0)
                                    {
                                        printf("%s ", command_args[i]);
                                    }
                                    else if (output_into_file == 1)
                                    {
                                        fprintf(file_write, "%s ", command_args[i]);
                                    }
                                }
                            }
                        }
                        if (no_of_commands - 1 == 0)
                        {
                            command_success = 0;
                        }
                    }
                    else if (strcmp(command_args[0], "alias") == 0) // alias
                    {

                        if (no_of_commands - 1 == 1)
                        {
                            command_success = 1;
                            int found = 0;
                            for (int i = 0; i < no_of_alias; i++)
                            {
                                if (strcmp(command_args[1], alias[i]) == 0)
                                {
                                    if (output_into_file == 0)
                                    {
                                        printf("%s", alias[i]);
                                        printf("=");
                                        printf("'%s'\n", alias_command[i]);
                                        found = 1;
                                        break;
                                    }
                                    else if (output_into_file == 1)
                                    {
                                        fprintf(file_write, "%s", alias[i]);
                                        fprintf(file_write, "=");
                                        fprintf(file_write, "'%s'\n", alias_command[i]);
                                        found = 1;
                                        break;
                                    }
                                }
                            }
                            if (found == 0)
                            {
                                command_success = 0;
                            }
                        }
                        else if (no_of_commands - 1 == 2)
                        {
                            command_success = 1;
                            no_of_alias = add_in_alias(alias, alias_command, no_of_alias, command_args);
                        }
                        else if (no_of_commands - 1 == 0)
                        {
                            command_success = 1;
                            for (int i = 0; i < no_of_alias; i++)
                            {
                                if (output_into_file == 0)
                                {
                                    printf("%s", alias[i]);
                                    printf("=");
                                    printf("'%s'\n", alias_command[i]);
                                }
                                else if (output_into_file == 1)
                                {
                                    fprintf(file_write, "%s", alias[i]);
                                    fprintf(file_write, "=");
                                    fprintf(file_write, "'%s'\n", alias_command[i]);
                                }
                            }
                        }
                        else if (no_of_commands - 1 > 2)
                        {
                            command_success = 0;
                        }
                    }
                    else if (strcmp(command_args[0], "unalias") == 0)
                    {
                        if (no_of_commands - 1 == 1)
                        {
                            command_success = 1;
                            int found = 0;
                            for (int i = 0; i < no_of_alias; i++)
                            {
                                if (strcmp(alias[i], command_args[1]) == 0)
                                {
                                    found = 1;
                                    if (i == no_of_alias - 1)
                                    {
                                        for (int len = 0; len < 200; len++) // if alias is at end of list
                                        {
                                            char *end = "\0";
                                            alias[i][len] = end[0];
                                            alias_command[i][len] = end[0];
                                        }
                                        no_of_alias = no_of_alias - 1;
                                        break;
                                    }
                                    for (int z = i; z < no_of_alias - 1; z++) // if alias is not at end of list
                                    {
                                        for (int len = 0; len < 200; len++)
                                        {
                                            char *end = "\0";
                                            alias[z][len] = end[0];
                                            alias_command[z][len] = end[0];
                                        }

                                        for (long unsigned int len_1 = 0; len_1 < strlen(alias[z + 1]); len_1++)
                                        {
                                            alias[z][len_1] = alias[z + 1][len_1];
                                        }

                                        for (long unsigned int len_2 = 0; len_2 < strlen(alias_command[z + 1]); len_2++)
                                        {
                                            alias_command[z][len_2] = alias_command[z + 1][len_2];
                                        }
                                    }
                                    for (int y = 0; y < 200; y++)
                                    {
                                        char *end = "\0";
                                        alias[no_of_alias - 1][y] = end[0];
                                        alias_command[no_of_alias - 1][y] = end[0];
                                    }
                                    no_of_alias = no_of_alias - 1;
                                    break;
                                }
                            }
                            if (found != 1)
                            {
                                command_success = 0;
                            }
                        }
                        else if (no_of_commands - 1 == 0 || no_of_commands >= 2)
                        {
                            command_success = 0;
                        }
                    }
                    else
                    {
                        char *commands_for_exec[100] = {NULL};
                        for (int i = 0; i < no_of_commands; i++)
                        {
                            commands_for_exec[i] = command_args[i];
                        }
                        if (pipeline_exists == 1)
                        {
                            int fd[2];
                            if (pipe(fd) == -1)
                            {
                                continue;
                            }

                            pid_t pid = fork();

                            if (pid == -1)
                            {
                                exit(1);
                            }
                            else if (pid == 0)
                            {
                                if (output_into_file == 1)
                                {
                                    if (dup2(fileno(file_write), 1) == -1)
                                    {
                                        exit(1);
                                    }
                                }
                                if (input_from_file == 1)
                                {
                                    if (dup2(fileno(file_input), 0) == -1)
                                    {
                                        exit(1);
                                    }
                                }

                                close(fd[0]);
                                if (STDIN_FILENO != in)
                                {
                                    if (dup2(in, STDIN_FILENO) == -1)
                                    {
                                        exit(1);
                                    }
                                    close(in);
                                }

                                if (STDOUT_FILENO != fd[1])
                                {
                                    if (dup2(fd[1], STDOUT_FILENO) == -1)
                                    {
                                        exit(1);
                                    }
                                    close(fd[1]);
                                }

                                int execstatus = execvp(command_args[0], commands_for_exec);
                                if (execstatus == -1)
                                {
                                    exit(1);
                                }
                            }
                            else
                            {
                                command_success = 1;
                                int wait_for_child;
                                wait(&wait_for_child);
                                close(fd[1]);
                                in = fd[0];

                                int exitstatus = WEXITSTATUS(wait_for_child);
                                if (exitstatus == 1)
                                {
                                    command_success = 0;
                                    break;
                                }
                            }
                        }
                        else if (pipeline_exists == -1) // last pipeline command
                        {
                            pid_t pid = fork();

                            if (pid == -1)
                            {
                                exit(1);
                            }
                            else if (pid == 0)
                            {
                                if (output_into_file == 1)
                                {
                                    if (dup2(fileno(file_write), 1) == -1)
                                    {
                                        exit(1);
                                    }
                                }
                                if (input_from_file == 1)
                                {
                                    if (dup2(fileno(file_input), 0) == -1)
                                    {
                                        exit(1);
                                    }
                                }

                                if (STDIN_FILENO != in)
                                {
                                    if (dup2(in, STDIN_FILENO) == -1)
                                    {
                                        exit(1);
                                    }
                                    close(in);
                                }
                                int execstatus = execvp(command_args[0], commands_for_exec);
                                if (execstatus == -1)
                                {
                                    exit(1);
                                }
                            }
                            else
                            {
                                command_success = 1;
                                int wait_for_child;
                                wait(&wait_for_child);

                                int exitstatus = WEXITSTATUS(wait_for_child);
                                if (exitstatus == 1)
                                {
                                    command_success = 0;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            pid_t pid = fork();

                            if (pid == -1)
                            {
                                exit(1);
                            }
                            else if (pid == 0)
                            {
                                if (output_into_file == 1)
                                {
                                    if (dup2(fileno(file_write), 1) == -1)
                                    {
                                        exit(1);
                                    }
                                }
                                if (input_from_file == 1)
                                {
                                    if (dup2(fileno(file_input), 0) == -1)
                                    {
                                        exit(1);
                                    }
                                }
                                int execstatus = execvp(command_args[0], commands_for_exec);
                                if (execstatus == -1)
                                {
                                    exit(1);
                                }
                            }
                            else
                            {
                                command_success = 1;
                                int wait_for_child;
                                wait(&wait_for_child);

                                int exitstatus = WEXITSTATUS(wait_for_child);
                                if (exitstatus == 1)
                                {
                                    command_success = 0;
                                    break;
                                }
                            }
                        }
                    }
                    if (output_into_file == 1)
                    {
                        fclose(file_write);
                        output_into_file = 0;
                    }
                    if (input_from_file == 1)
                    {
                        fclose(file_input);
                        input_from_file = 0;
                    }
                } while (index_of_pipeline < index_of_and_or - 1);
                pipeline_exists = 0;
            } while (index_of_and_or < index_of_sequential - 1);
        } while (index_of_sequential < no_of_chaining_command);
        free(input);
    }
    if (read_file == 1)
    {
        fclose(file_read_pointer);
    }

    return 0;
}