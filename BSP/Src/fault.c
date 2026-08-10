/**
 ******************************************************************************
 * @file    fault.c
 * @brief   HardFault context capture + serial report.
 ******************************************************************************
 */

#include "fault.h"
#include "usart.h"

volatile FaultInfo_t g_fault;

static char Fault_HexNibble(uint32_t v)
{
    static const char digits[] = "0123456789ABCDEF";
    return digits[v & 0x0FU];
}

static char* Fault_AppendHex(char* p, uint32_t v)
{
    uint8_t i;

    for (i = 0U; i < 8U; i++)
    {
        *p++ = Fault_HexNibble(v >> 28U);
        v <<= 4U;
    }
    return p;
}

static char* Fault_AppendTag(char* p, const char* tag, uint32_t v)
{
    while (*tag != '\0')
    {
        *p++ = *tag++;
    }
    *p++ = '=';
    p = Fault_AppendHex(p, v);
    *p++ = ' ';
    return p;
}

static char* Fault_AppendCfsr(char* p, uint32_t cfsr)
{
    struct
    {
        uint32_t mask;
        const char* name;
    } flags[] = {
        {0x00000100U, "IBUS"},       {0x00000200U, "PRECISE"},   {0x00000400U, "BFARV"},
        {0x00000800U, "IMPRECISE"},  {0x00001000U, "UNSTK"},     {0x00002000U, "STKERR"},
        {0x00004000U, "LSP"},        {0x00010000U, "DIVBYZERO"}, {0x00020000U, "UNALIGN"},
        {0x00040000U, "NOCP"},       {0x00080000U, "INVPC"},     {0x00100000U, "INVSTATE"},
        {0x00200000U, "UNDEFINSTR"},
    };
    uint8_t i;
    uint8_t any = 0U;

    for (i = 0U; i < (uint8_t)(sizeof(flags) / sizeof(flags[0])); i++)
    {
        if ((cfsr & flags[i].mask) != 0U)
        {
            *p++ = any ? '|' : '[';
            while (*flags[i].name != '\0')
            {
                *p++ = *flags[i].name++;
            }
            any = 1U;
        }
    }
    if (any != 0U)
    {
        *p++ = ']';
    }
    else
    {
        *p++ = '?';
    }
    *p++ = '\r';
    *p++ = '\n';
    return p;
}

void FAULT_Handler(uint32_t* stack)
{
    static volatile uint8_t already_here = 0U;
    char msg[160];
    char* p;
    uint32_t len;

    /* Exception frame: r0, r1, r2, r3, r12, lr, pc, xpsr */
    g_fault.r0 = stack[0];
    g_fault.r1 = stack[1];
    g_fault.r2 = stack[2];
    g_fault.r3 = stack[3];
    g_fault.r12 = stack[4];
    g_fault.lr = stack[5];
    g_fault.pc = stack[6];
    g_fault.xpsr = stack[7];
    g_fault.psp = __get_PSP();
    g_fault.cfsr = SCB->CFSR;
    g_fault.hfsr = SCB->HFSR;
    g_fault.bfar = SCB->BFAR;   /* 0xE000ED38 */
    g_fault.mmfar = SCB->MMFAR; /* 0xE000ED34 */

    if (already_here != 0U)
    {
        for (;;)
        {
        } /* fault during fault reporting */
    }
    already_here = 1U;

    p = msg;
    *p++ = '\r';
    *p++ = '\n';
    p = Fault_AppendTag(p, "HARDFAULT PC", g_fault.pc);
    p = Fault_AppendTag(p, "LR", g_fault.lr);
    p = Fault_AppendTag(p, "CFSR", g_fault.cfsr);
    p = Fault_AppendTag(p, "BFAR", g_fault.bfar);
    p = Fault_AppendCfsr(p, g_fault.cfsr);
    len = (uint32_t)(p - msg);

    (void)HAL_UART_Transmit(&huart1, (uint8_t*)msg, (uint16_t)len, 100U);

    for (;;)
    {
    }
}
