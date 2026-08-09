/**
  ******************************************************************************
  * @file    syscalls.c
  * @brief   Minimal newlib system call stubs for a bare-metal target.
  *
  *          printf()/puts() output is retargeted in BSP/Src/usart.c (_write).
  ******************************************************************************
  */

#include <sys/stat.h>
#include <sys/types.h>

extern char _end;    /* first free RAM address after .bss (linker script) */
extern char _estack; /* top of RAM, i.e. initial stack pointer            */

static char *heap_end = 0;

/**
  * @brief Program termination; bare-metal target never returns to an OS.
  */
void _exit(int status)
{
    (void)status;
    while (1)
    {
    }
}

/**
  * @brief Close a file descriptor.
  */
int _close(int fd)
{
    (void)fd;
    return -1;
}

/**
  * @brief Reposition a file offset. Not needed for UART/stdio-only apps.
  */
int _lseek(int fd, int offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    return 0;
}

/**
  * @brief Read from a file descriptor. Not used; no stdin retarget yet.
  */
int _read(int fd, char *ptr, int len)
{
    (void)fd;
    (void)ptr;
    (void)len;
    return -1;
}

/**
  * @brief File status: report character devices as "terminal".
  */
int _fstat(int fd, struct stat *st)
{
    (void)fd;
    st->st_mode = S_IFCHR;
    return 0;
}

/**
  * @brief Query whether a file descriptor is a terminal.
  */
int _isatty(int fd)
  {
    (void)fd;
    return 1;
}

/**
  * @brief Send a signal to a process. Not applicable.
  */
int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    return -1;
}

/**
  * @brief Get the process ID.
  */
int _getpid(void)
{
    return 1;
}

/**
  * @brief Extend the heap, with a collision guard against the stack region.
  */
void *_sbrk(ptrdiff_t incr)
{
    char *prev;

    if (heap_end == 0)
    {
        heap_end = &_end;
    }

    prev = heap_end;
    if ((heap_end + incr) > &_estack)
    {
        return (void *)-1; /* heap would collide with the stack */
    }
    heap_end += incr;

    return prev;
}
