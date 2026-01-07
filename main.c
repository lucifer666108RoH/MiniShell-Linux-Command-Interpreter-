/*Name : Anudeep R
date : 17/12/25
project name : minishell
about: is the min version of the terminal */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"main.h"
int main()
{
    
    system("clear");
    char prompt_string[50] = "minishell$:";
    char input_string[50];
    scan_input(prompt_string,input_string);
    return 0;
}