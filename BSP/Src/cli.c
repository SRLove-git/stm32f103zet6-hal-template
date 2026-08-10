/**
 ******************************************************************************
 * @file    cli.c
 * @brief   Command-line parser core (no hardware dependencies).
 ******************************************************************************
 */

#include "cli.h"

#include <stdio.h>
#include <string.h>

#define CLI_VERSION "stm32f103zet6-hal-template v1.0"

static const CLI_Cmd_t* cli_cmds[CLI_MAX_CMDS];
static uint8_t cli_ncmds = 0U;
static char cli_line[CLI_LINE_MAX];
static uint8_t cli_len = 0U;

static void Cmd_Help(int argc, char* argv[])
{
    uint8_t i;

    (void)argc;
    (void)argv;

    printf("Available commands:\r\n");
    for (i = 0U; i < cli_ncmds; i++)
    {
        printf("  %-12s %s\r\n", cli_cmds[i]->name, cli_cmds[i]->help);
    }
}

static void Cmd_Version(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    printf("%s\r\n", CLI_VERSION);
}

static void CLI_HandleLine(char* line)
{
    char* argv[CLI_MAX_ARGS];
    int argc = 0;
    char* p = line;
    uint8_t i;

    /* Tokenize on spaces */
    while ((*p != '\0') && (argc < (int)CLI_MAX_ARGS))
    {
        while (*p == ' ')
        {
            p++;
        }
        if (*p == '\0')
        {
            break;
        }
        argv[argc++] = p;
        while ((*p != '\0') && (*p != ' '))
        {
            p++;
        }
        if (*p != '\0')
        {
            *p++ = '\0';
        }
    }

    if (argc == 0)
    {
        return;
    }

    for (i = 0U; i < cli_ncmds; i++)
    {
        if (strcmp(cli_cmds[i]->name, argv[0]) == 0)
        {
            cli_cmds[i]->fn(argc, argv);
            return;
        }
    }

    printf("Unknown command: %s (type 'help')\r\n", argv[0]);
}

void CLI_Init(void)
{
    static const CLI_Cmd_t help_cmd = {"help", "list commands", Cmd_Help};
    static const CLI_Cmd_t ver_cmd = {"ver", "show version", Cmd_Version};

    cli_ncmds = 0U;
    cli_len = 0U;
    (void)CLI_Register(&help_cmd);
    (void)CLI_Register(&ver_cmd);

    printf("\r\n%s - type 'help' for commands\r\n", CLI_VERSION);
    CLI_Prompt();
}

uint8_t CLI_Register(const CLI_Cmd_t* cmd)
{
    if (cli_ncmds >= CLI_MAX_CMDS)
    {
        return 0U;
    }
    cli_cmds[cli_ncmds++] = cmd;
    return 1U;
}

void CLI_Feed(uint8_t ch)
{
    if ((ch == '\r') || (ch == '\n'))
    {
        if (cli_len > 0U)
        {
            cli_line[cli_len] = '\0';
            CLI_HandleLine(cli_line);
            cli_len = 0U;
        }
        CLI_Prompt();
        return;
    }

    if ((ch == 0x08U) || (ch == 0x7FU))
    {
        if (cli_len > 0U)
        {
            cli_len--;
            printf("\b \b");
        }
        return;
    }

    if (cli_len < (CLI_LINE_MAX - 1U))
    {
        cli_line[cli_len++] = (char)ch;
        printf("%c", ch); /* local echo */
    }
}

void CLI_Prompt(void)
{
    printf("stm32> ");
}
