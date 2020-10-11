/*
 * file:        part-2.c
 * description: Part 2, CS5600 load-and-switch assignment, Fall 2020
 */

/* NO OTHER INCLUDE FILES */
#include "elf64.h"
#include "sysdefs.h"

/* Definitions */
#define EOF    (-1)    // standard value for end of file
#define STDIN_FILE_DESCRIPTOR_NUMBER    0    // standard value for input file descriptor 
#define STDOUT_FILE_DESCRIPTOR_NUMBER    1    // standard value for output file descriptor 
#define STDERROR_FILE_DESCRIPTOR_NUMBER    2    // standard value for error file descriptor 
#define MAX_BUFFER_SIZE    200    // maximum size to do read and write

/* Open file mode flags definitions */
#ifndef O_RDONLY
    #define O_RDONLY    00000000
#endif
#ifndef O_WRONLY
    #define O_WRONLY    00000001
#endif
#ifndef O_RDWR
    #define O_RDWR      00000002
#endif

/* lseek file flags definitions */
#ifndef SEEK_SET
    #define SEEK_SET 0    /* set file offset to offset */
#endif
#ifndef SEEK_CUR
    #define SEEK_CUR 1    /* set file offset to current plus offset */
#endif
#ifndef SEEK_END
    #define SEEK_END 2    /* set file offset to EOF plus offset */
#endif

/* Error code definitions */
#define ERROR_NULL_POINTER    (-1000)

/* Exit code definitions */
#define EXIT_SUCCESS    0
#define EXIT_FAILURE    -1

/* function exit code definitions */
#define FUNCTION_SUCCESS    (EXIT_SUCCESS)
#define FUNCTION_FAILURE    (EXIT_FAILURE)

#define FD_SOFTLIMIT    1024
#define FD_HARDLIMIT    65535
#define FD_VALID_CHECK(fd)    ((fd > STDERROR_FILE_DESCRIPTOR_NUMBER) && (fd <= FD_SOFTLIMIT))

extern void *vector[];

/* ---------- */

/* write these functions 
 */
int read(int fd, void *ptr, int len);
int write(int fd, void *ptr, int len);
void exit(int err);
int open(char *path, int flags);
int close(int fd);
int lseek(int fd, int offset, int flag);
void *mmap(void *addr, int len, int prot, int flags, int fd, int offset);
int munmap(void *addr, int len);

/* ---------- */

/* the three 'system call' functions - readline, print, getarg 
 * hints: 
 *  - read() or write() one byte at a time. It's OK to be slow.
 *  - stdin is file desc. 0, stdout is file descriptor 1
 *  - use global variables for getarg
 */

void do_readline(char *buf, int len);
void do_print(char *buf);
char *do_getarg(int i);         

/* ---------- */

/* the guts of part 2
 *   read the ELF header
 *   for each section, if b_type == PT_LOAD:
 *     create mmap region
 *     read from file into region
 *   function call to hdr.e_entry
 *   munmap each mmap'ed region so we don't crash the 2nd time
 */

/* your code here */
int read(int fd, void *ptr, int len)
{
    int ret = FUNCTION_FAILURE;
    int readLength = 0;
    char *cPtr = (char*) ptr;
    char c;

    // with length of 0 returns zero and has no other effects
    if(len > 0) {
        // Read input character one at a time until the given length.
        do {
            syscall(__NR_read, fd, &c, 1);
            cPtr[readLength++] = c;
            ret = readLength;
        } while (c != '\n' && c != EOF && readLength < len);
        cPtr[readLength] = '\0';    // NULL terminate
    }
    else {
        ret = 0;
    }
    return ret;
}

int write(int fd, void *ptr, int len)
{
    int ret = FUNCTION_FAILURE;
    int writtenLength = 0;
    char *cPtr = (char*) ptr;
    char c = *(char*) ptr;
    
    // with length of 0 returns zero and has no other effects
    if(len > 0) {
        // Read input characters until EOF and newline character is found.
        while (c != NULL && writtenLength < len) {
            syscall(__NR_write, fd, &c, 1);
            ret = ++writtenLength;
            c = *(++cPtr);
        }
    }
    else {
        ret = 0;
    }
    return ret;
}

void exit(int err)
{
    syscall(__NR_exit, err);
}

int open(char *path, int flags)
{
    int ret = FUNCTION_FAILURE;
    if(path != NULL)
    {
        ret = syscall(__NR_open, flags);
    }
    // checks fd validity & returns file descriptor
    return FD_VALID_CHECK(ret) ? ret : FUNCTION_FAILURE;
}

int close(int fd)
{
    int ret = FUNCTION_FAILURE;
    if(FD_VALID_CHECK(fd))
    {
        ret = syscall(__NR_close, fd);
    }
    return ret;
}

int lseek(int fd, int offset, int flag)
{
    int ret = FUNCTION_FAILURE;
    if(FD_VALID_CHECK(fd))
    {
        ret = syscall(__NR_lseek, fd, offset, flag);
    }
    return ret; 
}

void *mmap(void *addr, int len, int prot, int flags, int fd, int offset)
{
    void* ret = FUNCTION_FAILURE;
    if((len > 0) && FD_VALID_CHECK(fd))
    {
        ret = syscall(__NR_mmap, addr, len, prot, flags, fd, offset); 
        if(ret == MAP_FAILED)
        {
            do_print("Mapping Failed\n");
        }
    }
    else
    {
        do_print("mmap input error\n");
    }
    return ret;
}

int munmap(void *addr, int len)
{
    int ret = FUNCTION_FAILURE;
    return ret;
}

int readline(char *pInput, int len)
{
    int ret = FUNCTION_FAILURE;
    if (pInput != NULL && len > 0) {
        // Read input character one at a time until EOF and newline character is found with NULL termination.
        ret = read(STDIN_FILE_DESCRIPTOR_NUMBER, pInput, len);
    }
    return ret;
}

void do_readline(char *buf, int len)
{
    readline(buf, len);
}

int print(void *pInput)
{
    int ret = FUNCTION_FAILURE;
    if (pInput != NULL) {
        // Read input characters until EOF and newline character is found.
        ret = write(STDOUT_FILE_DESCRIPTOR_NUMBER, pInput, MAX_BUFFER_SIZE);
    }
    return ret;
}

void do_print(char *buf)
{
    print((void *) buf);
}

char *do_getarg(int i)
{
    char* outArg;
    // gotta defn

    return outArg;
}

/* simple function to split a line:
 *   char buffer[200];
 *   <read line into 'buffer'>
 *   char *argv[10];
 *   int argc = split(argv, 10, buffer);
 *   ... pointers to words are in argv[0], ... argv[argc-1]
 */
int split(char **argv, int max_argc, char *line)
{
    int i = 0;
    char *p = line;

    while (i < max_argc) {
        while (*p != 0 && (*p == ' ' || *p == '\t' || *p == '\n'))
            *p++ = 0;
        if (*p == 0)
            return i;
        argv[i++] = p;
        while (*p != 0 && *p != ' ' && *p != '\t' && *p != '\n')
            p++;
    }
    return i;
}

/* ---------- */

void main(void)
{
    vector[0] = do_readline;
    vector[1] = do_print;
    vector[2] = do_getarg;

    /* YOUR CODE HERE */
    exit(0);
}

