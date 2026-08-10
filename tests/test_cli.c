#include <stdio.h>
#include <string.h>

#include "cli.h"

static int test_calls = 0;
static int test_argc = 0;
static char test_argv0[16];
static char test_argv1[16];

static void TestCmd(int argc, char* argv[])
{
    test_calls++;
    test_argc = argc;
    strncpy(test_argv0, argv[0], sizeof(test_argv0) - 1);
    if (argc > 1)
    {
        strncpy(test_argv1, argv[1], sizeof(test_argv1) - 1);
    }
}

#define CHECK(cond, msg)                                       \
    do                                                         \
    {                                                          \
        if (cond)                                              \
        {                                                      \
            printf("PASS: %s\n", msg);                         \
        }                                                      \
        else                                                   \
        {                                                      \
            printf("FAIL: %s\n", msg);                         \
            fails++;                                           \
        }                                                      \
    } while (0)

int main(void)
{
    int fails = 0;
    static const CLI_Cmd_t test_cmd = {"testcmd", "test command", TestCmd};
    const char* line = "testcmd alpha beta\r";
    size_t i;

    CLI_Init();
    CHECK(CLI_Register(&test_cmd) == 1, "register command");

    for (i = 0; i < strlen(line); i++)
    {
        CLI_Feed((uint8_t)line[i]);
    }

    CHECK(test_calls == 1, "command dispatched once");
    CHECK(test_argc == 3, "argc = 3");
    CHECK(strcmp(test_argv0, "testcmd") == 0, "argv[0] = testcmd");
    CHECK(strcmp(test_argv1, "alpha") == 0, "argv[1] = alpha");

    /* Empty line + backspace do not crash */
    CLI_Feed('\r');
    CLI_Feed('x');
    CLI_Feed(0x08);
    CLI_Feed('\r');
    CHECK(test_calls == 1, "no spurious dispatch on empty/edited line");

    printf(fails == 0 ? "ALL PASS\n" : "SOME FAILURES\n");
    return fails;
}
