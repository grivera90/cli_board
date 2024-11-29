/**
*******************************************************************************
* @file           : cli.h
* @brief          : Description of header file
* @author         : Gonzalo Rivera
* @date           : 30/08/2023
*******************************************************************************
* @attention
*
* Copyright (c) <date> grivera. All rights reserved.
*
*/
#ifndef __CLI_H__
#define __CLI_H__
/******************************************************************************
        Includes
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
/******************************************************************************
        Constants
 ******************************************************************************/

/******************************************************************************
        Data types
 ******************************************************************************/
typedef enum
{
        CMD_SET_INSTANCE = 0,
        CMD_DIAGNOSTIC_SPN,
        CMD_FORCE_SPN,
        CMD_SET_OWN_SPN,
        CMD_FW_UPGRADE,
        CMD_FW_UPGRADE_FORCE,
        CMD_FW_UPGRADE_ABORT,
        CMD_HELP,
        CMD_MAX
} cmd_list_t;
/******************************************************************************
        Public function prototypes
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function to cli init. Internally init the dummy function callbacks for all cmd's.
 *
 * @return none
 */
void cli_parser_init(void);

/**
 * @brief This function receive a pointer to the cmd that was entry for cli to be parser internally and ejecute.
 * 
 * @param char *cmd: pointer to cmd received for cli input.
 * 
 * @return none.
 */
void cli_parse(char *cmd);

/**
 * @brief Function to set a callback for a specific command.
 * 
 * @param int cmd: cmd to associate a specific cmd with a callback function. See \p cmd_list_t in Data types section.
 * 
 * @param void (*func)(int , char **): function pointer. This funcion receive an int as argument count and char** to point a command array or argument pointer. 
 * 
 * @return int: 0 to OK 1 for Error.
 */
int cli_set_cmd_cb(int cmd, void (*func)(int , char **));

/**
 * @brief This function receive a pointer to the name a pipe line fifo use by CLI. This name should be the same use in the script "./read_cmd" in this case.
 * 
 * @param char *fifo: pointer to the name a fifo pipe line.
 * 
 * @return none.
 */
void cli_set_fifo_stream(char *fifo);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // EOF __CLI_H__
