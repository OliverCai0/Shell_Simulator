#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

/*
Printing function for debugging
*/
void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

/*
Handles error messages and exit
*/
void errorMsg(int exit_toggle)
{
    char error_message[30] = "An error has occurred\n";
    write(STDOUT_FILENO, error_message, strlen(error_message));
    if (exit_toggle)
    {
        exit(1);
    }
}

/*
 *Return args separated by whitespace,
 *Will handle the overflow (more than 512 and no new line character)
 */
char **lineSanitize(char *input_command, int *arg_number)
{
    char command[512];
    strcpy(command, input_command);
    char **args = (char **)malloc(sizeof(char *) * 256);
    if (args == NULL)
    {
        printf("it's over grover\n");
        exit(1);
    }

    int arg_pointer = 0;
    char *token = strtok(command, ";");
    while (token != NULL)
    {
        if (token[strlen(token) - 1] == '\n')
        {
            token[strlen(token) - 1] = '\0';
        }
        args[arg_pointer] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(args[arg_pointer], token);
        arg_pointer++;
        token = strtok(NULL, ";");
    }
    *arg_number = arg_pointer;
    return args;
}

/*
Change directory command
*/
void getDirectory()
{
    char buff[200];
    getcwd(buff, 200);
    myPrint(buff);
    myPrint("\n");
}

/*
Parses number of occurences for character c
*/
int occurences(char *str, char c)
{

    int count = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == c)
        {
            count += 1;
        }
    }
    return count;
}

/*
 * returns [cmd, flags, args]
 */
char **getTokens(char *command, int *num_of_tokens)
{
    char **tokens = (char **)malloc(sizeof(char *) * 100);
    char temp[512];
    char *a;
    char *b;
    char *a_pointer;
    char *b_pointer;
    strcpy(temp, command);
    int token_pointer = 0;
    for (a = strtok_r(temp, "\t", &a_pointer); a != NULL; a = strtok_r(NULL, "\t", &a_pointer))
    {
        for (b = strtok_r(a, " ", &b_pointer); b != NULL; b = strtok_r(NULL, " ", &b_pointer))
        {
            tokens[token_pointer] = (char *)malloc(sizeof(char) * (strlen(b) + 1));
            strcpy(tokens[token_pointer], b);
            token_pointer++;
            *num_of_tokens += 1;
        }
    }
    tokens[token_pointer] = NULL;
    return tokens;
}

/*
Handles advanced redirection operator
*/
char **getAdvRedirection(char *command, int *num_of_tokens, int *redirect_error)
{
    char **tokens = (char **)malloc(sizeof(char *) * 2);
    char *pointer;
    if (((pointer = strstr(command, ">+")) != NULL))
    {
        *num_of_tokens += 1;
        char temp[512];
        strcpy(temp, command);
        char *token = strtok(temp, ">+");
        if (token == NULL || (occurences(command, '>') > 1))
        {
            *redirect_error = 1;
            return tokens;
        }
        tokens[0] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(tokens[0], token);
        if (((token = strtok(NULL, ">+")) != NULL))
        {
            char *cleaned = strtok(token, " ");
            if (cleaned == NULL)
            {
                *redirect_error += 1;
                return tokens;
            }
            tokens[1] = (char *)malloc(sizeof(char) * (strlen(cleaned) + 1));
            strcpy(tokens[1], cleaned);
        }
        else
        {
            free(tokens[0]);
            *redirect_error = 1;
            return tokens;
        }
    }
    return tokens;
}

/*
Handles regular redirection operator
*/
char **getRegRedirection(char *command, int *num_of_tokens, int *redirect_error)
{
    char **tokens = (char **)malloc(sizeof(char *) * 2);
    char *pointer;
    if ((pointer = strstr(command, ">")) != NULL)
    {
        char temp[512];
        strcpy(temp, command);
        char *token = strtok(temp, ">");
        if (token == NULL || (occurences(command, '>') > 1))
        {
            *redirect_error = 1;
            return tokens;
        }
        tokens[0] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(tokens[0], token);
        if ((token = strtok(NULL, ">")) != NULL)
        {
            char *cleaned = strtok(token, " ");
            if (cleaned == NULL)
            {
                *redirect_error += 1;
                return tokens;
            }
            tokens[1] = (char *)malloc(sizeof(char) * (strlen(cleaned) + 1));
            strcpy(tokens[1], cleaned);
        }
        else
        {
            free(tokens[0]);
            *redirect_error = 1;
            return tokens;
        }
        *num_of_tokens += 1;
    }
    return tokens;
}

/*
Main parsing function
*/
int parseCommands(char **kargs, int batchfile, char *command, int num_of_args)
{
    char **args;
    char **reg;
    char **adv;
    int line_printed = 0;
    for (int i = 0; i < num_of_args; i++)
    {

        if (strlen(kargs[i]) <= 0)
        {
            break;
        }
        int advanced_redirection = 0;
        int regular_redirection = 0;
        int adv_error = 0;
        int reg_error = 0;
        adv = getAdvRedirection(kargs[i], &advanced_redirection, &adv_error);
        if (!advanced_redirection && !adv_error)
        {
            reg = getRegRedirection(kargs[i], &regular_redirection, &reg_error);
        }
        int num_of_tokens = 0;
        if (regular_redirection && !reg_error)
        {
            args = getTokens(reg[0], &num_of_tokens);
        }
        else if (advanced_redirection && !adv_error)
        {
            args = getTokens(adv[0], &num_of_tokens);
        }
        else
        {
            args = getTokens(kargs[i], &num_of_tokens);
        }
        if ((num_of_args > 1 || num_of_tokens > 0 || adv_error || reg_error) && batchfile && !line_printed)
        {
            myPrint(command);
            line_printed = 1;
        }
        if (num_of_tokens == 0)
        {
            if (regular_redirection || advanced_redirection)
            {
                myPrint(command);
                errorMsg(0);
            }
        }
        else if (adv_error || reg_error)
        {
            errorMsg(0);
        }
        else if ((strcmp(args[0], "cd") == 0 || strcmp(args[0], "pwd") == 0 || strcmp(args[0], "exit") == 0) && (regular_redirection || advanced_redirection))
        {
            errorMsg(0);
        }
        else
        {
            if (strcmp(args[0], "cd") == 0 || strcmp(args[0], "cd\n") == 0)
            {
                if (num_of_tokens > 2)
                {
                    errorMsg(0);
                }
                else if (num_of_tokens == 1)
                {
                    chdir(getenv("HOME"));
                }
                else
                {
                    if (chdir(args[1]) == -1)
                    {
                        errorMsg(0);
                    }
                }
            }
            else if (strcmp(args[0], "pwd") == 0 || strcmp(args[0], "pwd\n") == 0)
            {
                if (num_of_tokens > 1)
                {
                    errorMsg(0);
                }
                else
                {
                    getDirectory();
                }
            }
            else if (strcmp(args[0], "exit") == 0)
            {
                if (num_of_tokens > 1)
                {
                    errorMsg(0);
                }
                else
                {
                    exit(0);
                }
            }
            else
            {
                if (fork() == 0)
                {
                    if (regular_redirection && !advanced_redirection)
                    {
                        if (!access(reg[1], F_OK))
                        {
                            errorMsg(1);
                        }
                        int new_output = open(reg[1], O_RDWR | O_CREAT, 00700);
                        if (new_output == -1)
                        {
                            errorMsg(1);
                        }
                        dup2(new_output, STDOUT_FILENO);
                        if (execvp(args[0], args) == -1)
                        {
                            errorMsg(1);
                        }
                    }
                    else if (advanced_redirection)
                    {
                        int output = open(adv[1], O_RDWR, 00700);
                        int length = lseek(output, 0, SEEK_END);
                        lseek(output, 0, SEEK_SET);
                        char *buffer = (char *)malloc(sizeof(char) * (length + 1));
                        read(output, buffer, length);
                        lseek(output, 0, SEEK_SET);
                        int trunced = open(adv[1], O_RDWR | O_CREAT | O_TRUNC, 00700);
                        if (trunced == -1)
                        {
                            errorMsg(1);
                        }
                        else
                        {
                            dup2(trunced, STDOUT_FILENO);
                            int c;
                            if ((c = fork()) == 0)
                            {

                                if (execvp(args[0], args) == -1)
                                {
                                    errorMsg(1);
                                }
                            }
                            else
                            {
                                int child_status;
                                wait(&child_status);
                                lseek(STDOUT_FILENO, 0, SEEK_END);
                                write(STDOUT_FILENO, buffer, length);
                                exit(0);
                            }
                        }
                    }
                    else if (execvp(args[0], args) == -1)
                    {
                        errorMsg(1);
                    }
                }
                else
                {
                    int status;
                    wait(&status);
                }
            }
        }

        for (int f = 0; f < num_of_tokens; f++)
        {
            free(args[f]);
        }

        if (advanced_redirection && !adv_error)
        {
            free(adv[1]);
            free(adv[0]);
        }
        if (regular_redirection && !reg_error)
        {
            free(reg[0]);
            free(reg[1]);
        }
    }
    return 1;
}

/*
Run shell in either interactive or batch mode
*/
int main(int argc, char *argv[])
{
    char cmd_buff[514];
    char *pinput;
    int status;
    int batchfile = 0;
    if (argc > 1)
    {
        if (access(argv[1], F_OK) == -1)
        {
            errorMsg(1);
        }
        if ((batchfile = open(argv[1], O_RDONLY, 0666)) == -1)
        {
            exit(1);
        }
        dup2(batchfile, STDIN_FILENO);
    }
    else if (argc > 2)
    {
        errorMsg(1);
    }
    while (1)
    {
        int num_of_args = 0;
        wait(&status);
        if (!batchfile)
        {
            myPrint("myshell> ");
        }
        pinput = fgets(cmd_buff, 514, stdin);
        if (!pinput)
        {
            exit(0);
        }
        if ((pinput[strlen(pinput) - 1] != '\n'))
        {
            myPrint(pinput);
            while (cmd_buff[strlen(pinput) - 1] != '\n')
            {
                pinput = fgets(cmd_buff, 514, stdin);
                myPrint(pinput);
            }
            errorMsg(0);
        }
        else
        {

            char **args = lineSanitize(pinput, &num_of_args);
            if (num_of_args > 0)
            {
                parseCommands(args, batchfile, pinput, num_of_args);
            }

            for (int i = 0; i < 256; i++)
            {
                free(args[i]);
            }
        }
    }
}
