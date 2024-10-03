/**
* FILENAME: cli.h
*
* DESCRIPTION:
*
* In this file you will find the function headers
*
* AUTHOR: Muhammad Zaman 200449177
*/

#ifndef CLI_H
#define CLI_H

void uart_send_string(const char *str);
void cli_process_command(const char *cmd);
void cli_prompt(void);
void cli_run(void);
void cli_welcome(void);

#endif /* SRC_CLI_H_ */
