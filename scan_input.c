/*Name : Anudeep R
date : 17/12/25
project name : minishell
about: is the min version of the terminal */

#include<stdio.h>  // Standard input-output functions
#include<stdlib.h> // malloc, free, exit
#include<string.h> // string handling functions
#include <strings.h> // bzero()
#include"main.h"      // user-defined header file

char *external_cmd[156];   // Array to store list of external commands
char input_cmd[50];        // Stores the full input command (used for jobs)
slist *head = NULL;        // Head pointer for job linked list
int pid =-1;               // Stores current running child PID
int status = 0;            // Stores child process exit status
char prompt_cmd[50];         // Stores current shell prompt


// Function to scan user input continuously
void scan_input(char *prompt, char *input_string)
{
    strcpy(prompt_cmd,prompt);                // Save prompt for signal handler
    signal(SIGINT,signal_handler);            // Register Ctrl+C handler
    signal(SIGTSTP, signal_handler);         // Register Ctrl+Z handler
    signal(SIGCHLD,signal_handler);          // Register child termination handler

    extract_external_commands(external_cmd); // Load external commands list

    while(1)
    {
        bzero(input_string,50);                     // Clear input buffer
        printf("%s",prompt);                         // Display prompt
        fflush(stdout);
        int ret = scanf("%[^\n]",input_string);    // Read input until newline
        if(ret == EOF)                             // Ctrl+D detected
        {
            printf("\n");
            exit(0);                                // Exit shell
        }
        __fpurge(stdin);                           // Clear stdin buffer
        if(input_string[0] == '\0' )              // If empty input
        {
            
            continue;                            // Ignore and continue
        }
        if(strncmp(input_string,"PS1=",4) ==0)   // Prompt change command
        {
            copy_change(prompt,input_string);
            continue;
        }
        strcpy(input_cmd,input_string);     // Save command for job list
        
        char *cmd = get_command(input_string); // Extract first word(command)
        
        if(strcmp(cmd,"jobs") == 0)    // Built-in jobs command
        {
            print_list(head);          // Print job list
            continue;
        }
        if(strcmp(cmd,"fg") == 0)    // Foreground command
        {
            if(head == NULL)
            {
                printf("fg: current: no such job\n");
                continue;
            }
            printf("%s\n",head->cmd);                // Print job command
            kill(head->pid,SIGCONT);                 // Resume stopped process
            waitpid(head ->pid,&status,WUNTRACED);
            delete_first(&head);                     // Remove job from list
            continue;
        }
        if(strcmp(cmd,"bg")==0)                    // Background command
        {
            if(head == NULL)
            {
                printf("bg: current: no such job\n");
                continue;
            }
            printf("%s\n",head->cmd);
            kill(head->pid,SIGCONT);              // Resume in background
            delete_first(&head);
            continue;
        }
        int op = check_command_type(cmd);           // Check command type
        if(op == 1)                                 // Built-in command
        {
            execute_internal_commands(input_string,prompt);
        }
        else if(op == 2)                             // External command
        {
            execute_external_commands(input_string);
        }
        else                                          // Invalid command
        {
            printf("No Command\n");
        }
        free(cmd);                                   // Free allocated command
        
    }
    
}

// Function to change shell prompt
void copy_change(char *prompt, char *input_string)
{
    int len  = strlen(input_string);
    if(len == 4)                                        // Only PS1=
    {
        printf("No command!\n");
        return;
    }
    for (int i = 4; i< len; i++)                     // Check for spaces
    {
        if (input_string[i] == ' ')
        {
            printf("No command!\n");
            return;
        }
    }
    strcpy(prompt,input_string+4);                   // Copy new prompt
    strcpy(prompt_cmd,prompt);                       // Save for signal handler
}

// Extract first word (command) from input
char *get_command(char *input_string)
{
    char *cmd = malloc(50*sizeof(char));
    int k = 0,i = 0;
    while (input_string[i] == ' ')                   // Skip leading spaces
    {
        i++;
    }

    while (input_string[i] != ' ' && input_string[i] != '\0')
    {
        cmd[k++] = input_string[i++];                  // Copy command characters
    }

    cmd[k] = '\0';                                  // Null terminate
    return cmd;
}

// Check whether command is builtin or external
int check_command_type(char *command)
{
    char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
						"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
						"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", NULL};

    for(int i=0; builtins[i]!= NULL;i++)
    {
        if(strcmp(builtins[i],command)==0)
        {
            return BUILTIN;
        }
    }
    for(int i = 0;external_cmd[i]!= NULL;i++)
    {
        if(strcmp(external_cmd[i],command)==0)
        {
            return EXTERNAL;
        }
    }

    return NO_COMMAND;
}

// Read external commands from file
void extract_external_commands(char **external_commands)
{
    char buffer[50];
    int i;
    FILE *fp = fopen("external.txt","r");
    for(i = 0;i<155;i++)
    {
        if (fgets(buffer, sizeof(buffer), fp) == NULL)
        {
            break;
        }
        
        buffer[strcspn(buffer, "\n")] = '\0';    // Remove newline

        external_cmd[i] = malloc(strlen(buffer) + 1);
        strcpy(external_cmd[i], buffer);
    }
    external_cmd[i] = NULL;
}

// Execute built-in commands
void execute_internal_commands(char *input_string,char * prompt)
{
    char *cmd = get_command(input_string);
    if(strcmp(cmd,"exit")==0)                   // Exit shell
    {
        exit(0);
    }
    if(strcmp(cmd,"pwd")==0)                // Print working directory
    {
        char cwd[100];
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
        free(cmd);
        return;
        
    }
    if(strcmp(cmd,"cd")==0)                    // Change directory
    {
        char *dir = input_string +strlen(cmd);  // skip "cd"
        char cwd[100];
        while (*dir == ' ')
            dir++; //skip spaces

        if (*dir == '\0')
        {
            dir = getenv("HOME");
            /*getcwd(cwd, sizeof(cwd));
            sprintf(prompt, "%s$ ", cwd);*/
        }
              

        if (chdir(dir) == -1)                    // Change directory
        {
            printf("cd: no such file or directory: %s\n", dir);
        }
       /* else
        {
            getcwd(cwd, sizeof(cwd));
            sprintf(prompt, "%s$ ", cwd);
        }*/
            

        free(cmd);
        return;
    }
    int len = strlen(input_string);
    if(strcmp(cmd,"echo") == 0)                      // Echo command
    {
        char *shell = malloc(50);
        if( len>= 6 && strncmp(input_cmd + len -6 ,"$SHELL",6) == 0)
        {
                
            shell = getenv("SHELL");
            printf("%s\n",shell);
            return; 
        }
            
        if(len >= 2 && strncmp(input_string + len -2 ,"$?",2)==0)
        {
            printf("%d\n",status);                                  // Print last status
            return;
        }
        if(len >= 2 && strncmp(input_string + len -2 ,"$$",2)==0)
        {
            printf("%d\n",getpid());                             // Print shell PID
            return;
        }
        strcpy(shell,input_string+4);
        printf("%s\n",shell);                                  // Print echo string
        free(shell);
        free(cmd);
        return;
    }
    free(cmd);
}

// Execute external commands (with pipes)
void execute_external_commands(char *input_string)
{
    char cmd[50];
    strcpy(cmd,input_string);
    char *argv[30];
    int count = 0;
    char * word = strtok(cmd," ");
    while(word != NULL)
    {
        argv[count] = word;
        count++;
        word = strtok(NULL," ");
    }
    argv[count] = NULL;

    int cmd_ind[count],k = 1;
    int cmd_count = 1;
    int pipe_found = 0;
    cmd_ind[0] = 0;
    for(int i = 0;i<count;i++)
    {
        if(strcmp(argv[i],"|")==0)
        {
            pipe_found = 1;
            argv[i] = NULL;
            cmd_ind[k++] = i+1;
            cmd_count ++;
        }
    }
    if(!pipe_found)                     // No pipe case
    {
        pid = fork();
        if(pid == 0)
        {
            signal(SIGTSTP, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            execvp(argv[0],argv);
        }
        else if(pid >0)
        {
            waitpid(pid, &status,WUNTRACED);
            pid = -1;
            return;
        }
        
    }
    int fd[2];
    int backup_stdout = dup(1);
    int backup_stdin = dup(0);
    for(int i = 0;i<cmd_count;i++)
    {
        if(i != cmd_count-1 )
        {
            pipe(fd);
        }
        pid = fork();
        if(pid==0)
        {
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            if(i!=cmd_count-1)
            {
                close(fd[0]);
                dup2(fd[1],1);
            }
            execvp(argv[cmd_ind[i]],argv+cmd_ind[i]);
            printf("Error:execvp failed\n ");
        }
        else if(pid > 0)
        {
            
            if(i!=cmd_count-1)
            {
                close(fd[1]);
                dup2(fd[0],0);
                close(fd[0]);

            }
        }
    }
    while(waitpid(-1,&status,WUNTRACED)>0); // Wait for all children
    pid = -1;
   
    dup2(backup_stdin,0);    // Restore stdin
    dup2(backup_stdout,1);   // Restore stdout
}

// Signal handler
void signal_handler(int sig_num)
{
    if(sig_num == SIGINT)                // Ctrl+C
    {
        if(pid == -1)
        {
          write(1, "\n", 1);
          write(1, prompt_cmd, strlen(prompt_cmd));
          
        }
       
    }
    if(sig_num == SIGTSTP)                 // Ctrl+Z
    {
       
        if(pid !=-1)
        {
            
            insert_first(&head,pid,input_cmd);
            printf("\n Stopped                 %s\n", input_cmd);
            //fflush(stdout);
        }
        else if(pid == -1)
        {
           
           
            write(1, "\n", 1);
            write(1, prompt_cmd, strlen(prompt_cmd));
           
        }
         
    }
    if(sig_num == SIGCHLD)                          // Child terminated
    {
        while(waitpid(-1,&status,WNOHANG)>0);
    }
}

// Insert job at beginning of list
int  insert_first(slist **head,int pid, char *cmd)
{
    slist *new =  malloc(sizeof(slist));
    if(new == NULL)
    {
        return -1;
    }
    new ->pid = pid;
    strcpy(new->cmd,cmd);
    new ->link = *head;
    *head = new;
    return 0;
}

// Print job list
void print_list(slist *head)
{
    if (head == NULL)
    {
        printf("jobs: current: no such job\n");
        return;
    }

    while (head)
    {
        printf("Stopped       %s\n", head->cmd);
        head = head->link;
    }
    
}

// Delete first job from list
void delete_first(slist **head)
{
    if (*head == NULL)
    {
       return;
    }
    slist *temp = *head;
    *head = (*head)->link;
    free(temp);
}