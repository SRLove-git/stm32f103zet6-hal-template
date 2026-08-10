/**
 ******************************************************************************
 * @file    cli.h
 * @brief   Tiny line-based command-line interface (core is hardware-free).
 *
 *          Feed bytes with CLI_Feed() (e.g. from the USART1 RX ring buffer);
 *          complete lines are tokenized and dispatched to registered commands.
 ******************************************************************************
 */

#ifndef __CLI_H
#define __CLI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define CLI_LINE_MAX 80U
#define CLI_MAX_CMDS 16U
#define CLI_MAX_ARGS 10U

    typedef void (*CLI_CmdFn)(int argc, char* argv[]);

    typedef struct
    {
        const char* name;
        const char* help;
        CLI_CmdFn fn;
    } CLI_Cmd_t;

    /**
     * @brief Initialize the CLI (banner + built-in commands + prompt).
     */
    void CLI_Init(void);

    /**
     * @brief Register a command.
     * @retval 1 on success, 0 if the command table is full.
     */
    uint8_t CLI_Register(const CLI_Cmd_t* cmd);

    /**
     * @brief Feed one received byte (line buffer + dispatch on newline).
     */
    void CLI_Feed(uint8_t ch);

    /**
     * @brief Print the prompt.
     */
    void CLI_Prompt(void);

#ifdef __cplusplus
}
#endif

#endif /* __CLI_H */
