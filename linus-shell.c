

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>   // exit()
#include <unistd.h>   // fork(), getpid(), exec()
#include <sys/wait.h> // wait()
#include <signal.h>   // signal()
#include <fcntl.h>    // close(), open()
#define LSH_RL_BUFSIZE 1024

char **get_input(char *input)
{
    char **command = malloc(8 * sizeof(char *));
    char *separator = " ";
    char *parsed;
    int index = 0;

    // Splits input according to given delimiters.
    parsed = strtok(input, separator);
    while (parsed != NULL)
    {
        command[index] = parsed;
        index++;

        parsed = strtok(NULL, separator);
    }

    command[index] = NULL;
    return command;
}

char *trimwhitespace(char *str)
{
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

int parseInput(char **command)
{

    size_t ln = strlen(command[0]) - 1;
    if (*command[0] && command[0][ln] == '\n')
        command[0][ln] = '\0';

    if (strcmp(command[0], "exit") == 0)
    {

        free(command);
        return 1;
    }

    int iterator = 0, dflag = 0;
    while (command[iterator])
    {
        if (strcmp(command[iterator], "&&") == 0)
        {
            dflag = 1;
            break;
        }
        iterator++;
    }

    if (dflag == 1)
    {
        return 2;
    }

    iterator = 0;
    dflag = 0;

    while (command[iterator])
    {
        if (strcmp(command[iterator], "##") == 0)
        {
            dflag = 1;
            break;
        }
        iterator++;
    }

    if (dflag == 1)
    {
        return 3;
    }

    iterator = 0;
    dflag = 0;

    while (command[iterator])
    {
        if (strcmp(command[iterator], ">") == 0)
        {
            dflag = 1;
            break;
        }
        iterator++;
    }

    if (dflag == 1)
    {
        return 4;
    }

    // This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
}

void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s$", cwd);
}

int cd(char *path)
{
    return chdir(path);
}

void executeCommand(char **command)
{
    // This function will fork a new process to execute a command
    if (strcmp(command[0], "cd") == 0)
    {
        //Error handling
        if (cd(command[1]) < 0)
        {
            printf("Shell: Incorrect command\n");
        }
        /* Skips the fork, as child process is no longer required */
        return;
    }

    else
    {
        int rc = fork();
        if (rc == 0)
        {
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            if (execvp(command[0], command) == -1)
            {
                printf("Shell: Incorrect command\n");
            }
            exit(0);
        }

        else if (rc > 0)
        {
            wait(0);
        }

        else
        {
            printf("There's an error while forking\n");
        }
    }
}

void executeParallelCommands(char **command)
{
    int iterator = 0;
    char *temp[100][100];
    int i = 0, j = 0, k = 0;
    pid_t pidChild = 1;
    int status;
    for (iterator = 0; command[i] != NULL; iterator++)
    {
        if (strcmp(command[i], "&&") != 0)
        {

            temp[j][k] = command[i];
            i++;
            k++;
        }

        else
        {
            temp[j][k] = NULL;
            i++;
            j++;
            k = 0;
        }
    }

    temp[j][k] = NULL;
    temp[j + 1][0] = NULL;

    for (i = 0; temp[i][0] != NULL && pidChild != 0; i++)
    {
        pidChild = fork();
        if (pidChild < 0)
        {
            printf("Shell: Incorrect command\n");
            exit(1);
        }

        else if (pidChild == 0)
        {
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            execvp(temp[i][0], temp[i]);
        }
    }

    while (j >= 0)
    {
        waitpid(-1, &status, WUNTRACED);
        j--;
    }
    // This function will run multiple commands in parallel
}

void executeSequentialCommands(char **command)
{

    int iterator = 0;
    int i = 0, j = 0, k = 0;
    char *temp[30];
    for (iterator = 0; command[i] != NULL; iterator++)
    {
        if (strcmp(command[i], "##") == 0)
        {
            temp[k] = NULL;
            executeCommand(temp);
            i++;
            k = 0;
        }

        else
        {
            temp[k] = command[i];
            i++;
            k++;
        }
    }

    temp[k] = NULL;
    executeCommand(temp);

    // This function will run multiple commands in parallel
}

void executeCommandRedirection(char **command)
{
    int count = 0;
    char **temp = command;
    while (*temp++)
        count++;

    int i = 0;
    char **tobeexecute = malloc(8 * sizeof(char *));
    char *file;
    while (command[i] != NULL)
    {
        if (strcmp(command[i], ">") == 0)
        {
            file = command[i + 1];
            break;
        }
        tobeexecute[i] = command[i];
        i++;
    }
    // printf("file is %s",file);

    int fp = open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
    int output = dup(1);
    dup2(fp, 1);
    executeCommand(tobeexecute);
    dup2(output, 1);

    // This function will run a single command with output redirected to an output file specificed by user
}

int main()
{
    // Initial declarations

    // char *input;
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    while (1) // This loop will keep your shell running until user exits.
    {

        char **command;
        // Print the prompt in format - currentWorkingDirectory$
        printDir();
        // accept input with 'getline()'
        size_t buffer_size = 80;
        char *input = malloc(buffer_size);

        getline(&input, &buffer_size, stdin);
        input = trimwhitespace(input);
        command = get_input(input);
        // printf("Isme dekho kya kya hai\n");
        // char **temp = command;
        // while (*temp)
        // printf("%s\n", *temp++);

        // Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
        int ret_val = parseInput(command);

        if (ret_val == 1) // When user uses exit command.
        {
            printf("Exiting shell...\n");
            break;
        }

        if (ret_val == 2)
        {
            // printf("Idharich hai apun && wala \n");
            executeParallelCommands(command); // This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
        }

        else if (ret_val == 3)
        {
            // printf("Idharich hai apun ## wala \n");
            executeSequentialCommands(command); // This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
        }

        else if (ret_val == 4)
        {
            // printf("Idharich hai apun > wala \n");
            executeCommandRedirection(command); // This function is invoked when user wants redirect output of a single command to and output file specificed by user
        }

        else
        {
            // printf("Ye single command hai re \n");
            executeCommand(command); // This function is invoked when user wants to run a single commands
        }

        free(input);
        free(command);
    }

    return 0;
}