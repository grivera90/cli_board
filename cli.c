/**
*******************************************************************************
* @file           : cli.c
* @brief          : Description of C implementation module
* @author         : Gonzalo Rivera
* @date           : 30/08/2023
*******************************************************************************
* @attention
*
* Copyright (c) <date> grivera. All rights reserved.
*
*/
/******************************************************************************
    Includes
******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cli.h"
/******************************************************************************
    Defines and constants
******************************************************************************/
#define CMD(func, short_name, params, help) {#func, #short_name, cmd_ ## func, params, help}
#define BUFFER_LEN 512
static const char *delim = " \n(,);";
static const char *vboard_str = "vboard";
static char buffer[BUFFER_LEN] = { 0 };
/******************************************************************************
    Data types
******************************************************************************/
typedef struct
{
    const char *name;
    const char *short_name;
    void (*cmd_func)(int argc, char **);
    const int argc;
    const char *doc;
} cmd_t;

/******************************************************************************
    Local variables
******************************************************************************/
static cmd_t cmd_table[CMD_MAX] = 
{
    {"set_instance", "-i", NULL, 3, "Set a instance of an ECU. vboard -i <type>,<identity>,<instance>.\n\t\ta) vboard -i 10,200,0.\n\t\tb) vboard -i 10,0:5,2:7."},
    {"diag_spn", "-d", NULL, 4, "Send a spn at set period. vboard -d <da>,<spn>,<period>,<-e/-d>.\n\t\ta) vboard -d 22,2838,200,-e (da = 22, spn = 2838, period = 200, enable)."},
    {"force_spn", "-f", NULL, 4, "Force a spn value. vboard -f <da>,<spn>,<value>,<-e/-d>.\n\t\ta) vboard -f 22,2838,xxxx,-e (da = 22, spn = 2838, value = xxxx, enable)."},
    {"set_own_spn", "-s", NULL, 3, "Set own spn value. vboard -s <spn>,<spn_value>,<value_type>.\n\t\ta) vboard -s 520198,1200,-lld. For integers values.\n\t\tb) vboard -s 520198,126.55,-f. For float values.\n\t\tc) vboard -s 520198,-735.28,-lf. For double values.\n\t\td) vboard -s 520198,654090,-r. For raw values."},
    {"fw_upgrade", "-fw", NULL, 2, "Upgrade the firmware of a specific ecu/node. vboard -fw </path_to_fw/binary.bin> <address>."},
    {"fw_upgrade_force", "-fwf", NULL, 2, "Upgrade the firmware of a specific ecu/node without version check. vboard -fw </path_to_fw/binary.bin> <address>."},
    {"fw_upgrade_abort", "-fwa", NULL, 0, "Upgrade firmware abort."},
    {"help", "-h", NULL, 0, "Display this help."}
};

static char **args_vector;
static char *fifo_name = NULL;
/******************************************************************************
    Local function prototypes
******************************************************************************/
/**
 * @brief Function to return an element split for the "delim". 
 * 
 * @param int argc: current argument.
 * 
 * @return char **. pointer to the argument that will be parser latter.
 */
static char **args_parse(int argc);

/**
 * @brief Function dummy to show the cmd entry. 
 * 
 * @param int argc: argument count.
 * 
 * @param char **: pointer to the arguments.
 * 
 * @return none.
 */
static void cmd_dummy(int argc, char **args);

/**
 * @brief Function to print the help.
 * 
 * @param int argc: current argument.
 * 
 * @param char **: pointer to the argument.
 * 
 * @return none.
 */
static void cmd_help(int argc, char **args);
/******************************************************************************
    Local function definitions
******************************************************************************/
#define ESCAPE {free(args); puts("Bad Argument(s)"); return NULL;}
static char **args_parse(int argc)
{
    int i;
    char **args = malloc(sizeof(char **)*argc);

    for(i = 0; i < argc; ++i)
    {
        args[i] = strtok(NULL, delim);
        if(!args[0])
        {
            ESCAPE;
        }
    }

    return args;
}
#undef ESCAPE

static void cmd_dummy(int argc, char **args)
{
    memset(buffer, 0, sizeof(buffer));

    sprintf(buffer, "cmd_dummy: ");

    for(int i = 0; i < argc; i++)
    {
        strcat(buffer, args[i]);
        if(i < argc - 1)
        {
            strcat(buffer, ",");
        }
    }

    printf("%s\n", buffer);
}

static void cmd_help(int argc, char **args)
{
    int i = CMD_MAX;
    int len = 0;
    int fd = open(fifo_name, O_WRONLY);
    
    len = sprintf(buffer, "Available Commands:\n");
    write(fd, buffer, len + 1);

    for(i = 0; i < CMD_MAX; i++)
    {
        cmd_t cmd = cmd_table[i];    
        len = sprintf(buffer, "%20s (%s) - %s\n", cmd.name, cmd.short_name, cmd.doc);
        write(fd, buffer, len + 1);      
    }

    close(fd);
}

/******************************************************************************
    Public function definitions
******************************************************************************/
void cli_parser_init(void)
{
    cmd_table[CMD_SET_INSTANCE].cmd_func = cmd_dummy;
    cmd_table[CMD_DIAGNOSTIC_SPN].cmd_func = cmd_dummy;
    cmd_table[CMD_FORCE_SPN].cmd_func = cmd_dummy;
    cmd_table[CMD_SET_OWN_SPN].cmd_func = cmd_dummy;
    cmd_table[CMD_FW_UPGRADE].cmd_func = cmd_dummy;
    cmd_table[CMD_FW_UPGRADE_FORCE].cmd_func = cmd_dummy;
    cmd_table[CMD_FW_UPGRADE_ABORT].cmd_func = cmd_dummy;
    cmd_table[CMD_HELP].cmd_func = cmd_help;   
    cli_set_fifo_stream("/tmp/pipe_rx");
}

void cli_parse(char *cmd)
{
    int i = CMD_MAX;
    char* tok = strtok(cmd, delim);
    
    if(!tok)
    {
        return;
    }

    if(!strcmp(tok, vboard_str))
    {
        tok = strtok(NULL, delim);
        while(i--) 
        {
            cmd_t cur = cmd_table[i];
            if(NULL == tok)
            {
                break;
            }

            if(!strcmp(tok, cur.name) || !strcmp(tok, cur.short_name)) 
            {
                args_vector = args_parse(cur.argc);
                if(args_vector == NULL)
                {
                    return; //Error in argument parsing.
                }

                if(cur.cmd_func != NULL)
                {
                    cur.cmd_func(cur.argc, args_vector);
                }
                
                free(args_vector);
                return;
            }
        }        
    }
    
    puts("Command Not Found");
}

int cli_set_cmd_cb(int cmd, void (*func)(int , char **))
{
    if(cmd >= CMD_MAX || NULL == func)
    {
        return EXIT_FAILURE;
    }

    cmd_table[cmd].cmd_func = func;

    return EXIT_SUCCESS;    
}

void cli_set_fifo_stream(char *fifo)
{
    if(fifo != NULL)
    {
        fifo_name = fifo;
    }
}
