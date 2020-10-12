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
#define MAX_ARGUMENTS    10    // max number of input arguments 
#define PAGE_SIZE    4096    // size of the virtual page

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
#define ERROR_INVALID_INDEX    (-1001)

/* Exit code definitions */
#define EXIT_SUCCESS    0
#define EXIT_FAILURE    -1

/* function exit code definitions */
#define FUNCTION_SUCCESS    (EXIT_SUCCESS)
#define FUNCTION_FAILURE    (EXIT_FAILURE)

#define FD_SOFTLIMIT    1024
#define FD_HARDLIMIT    65535
#define FD_VALID_CHECK(fd)    ((fd > STDERROR_FILE_DESCRIPTOR_NUMBER) && (fd <= FD_SOFTLIMIT))

/* round A up to the next multiple of B */
#define ROUND_UP(a,b)    (((a+b-1)/b)*b)

#define LOAD_START_ADDR    0x80000000

extern void *vector[];

/* ---------- */
/* Global variables */
char *argv[MAX_ARGUMENTS];
int argc;
void (*func)();

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
        ret = syscall(__NR_open, path, flags);
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
    void* ret = (void*) FUNCTION_FAILURE;
    if((len > 0))
    {
        ret = (void*) syscall(__NR_mmap, addr, len, prot, flags, fd, offset); 
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
    if(len > 0)
    {
        ret = syscall(__NR_munmap, addr, len);
    }
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

void print(char *buf)
{
    // int ret = FUNCTION_FAILURE;
    if (buf != NULL) {
        // Read input characters until EOF and newline character is found.
        // ret = write(STDOUT_FILE_DESCRIPTOR_NUMBER, buf, MAX_BUFFER_SIZE);
        write(STDOUT_FILE_DESCRIPTOR_NUMBER, buf, MAX_BUFFER_SIZE);
    }
    // return ret;
}

void do_print(char *buf)
{
    print(buf);
}

char *do_getarg(int i)
{
    if(i < argc)
        return argv[i];
    else
        return NULL;         
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

int exec_file(char *filename)
{
    int ret = FUNCTION_FAILURE;
    int fd;
    
    if ((fd = open(filename, O_RDONLY)) < 0) {
        do_print("file open failed\n");
        return ret;		/* failure code */
    }

    /* read the main header (offset 0) */
    struct elf64_ehdr hdr;
    read(fd, &hdr, sizeof(hdr));
    
    /* read program headers (offset 'hdr.e_phoff') */
    int i, n = hdr.e_phnum;
    struct elf64_phdr phdrs[n];
    lseek(fd, hdr.e_phoff, SEEK_SET);
    read(fd, phdrs, sizeof(phdrs));

    void* addr_list[n];
    int length[n];
    int index = 0;
    
    /* look at each section in program headers */
    for (i = 0; i < hdr.e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
	        int len = ROUND_UP(phdrs[i].p_memsz, PAGE_SIZE);
            void *buf = mmap(phdrs[i].p_vaddr + LOAD_START_ADDR, len, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if(buf == MAP_FAILED) {
                do_print("couldn't mmap\n");
                exit(-1);
            }
            lseek(fd, (int)phdrs[i].p_offset, SEEK_SET);
            read(fd, buf, (int)phdrs[i].p_filesz);
            addr_list[index] = buf;
            length[index] = (int)phdrs[i].p_filesz;
            index++;
	    }
    }
    
    func = hdr.e_entry + LOAD_START_ADDR;
    func();
    --index;
    while(index >= 0) {
        int unmap_result = munmap((void *) addr_list[index], length[index]);
        if (unmap_result != 0) {
            do_print("Could not munmap");
            exit(-1);
        }
        index--;
    }

    close(fd);

    ret = FUNCTION_SUCCESS;
    return ret;
}

void main(void)
{
    vector[0] = do_readline;
    vector[1] = do_print;
    vector[2] = do_getarg;

    char input[MAX_BUFFER_SIZE] = {0};
    char *pInput = &input[0];
    char quit[5] = {'q', 'u', 'i', 't', '\n'};
    char *pQuit = &quit[0];
    int exit_code = EXIT_FAILURE;

    if(pInput != NULL) {    // NULL check, if in case malloc is used in future
        while(1) {
            do_print("> ");
            readline(pInput, MAX_BUFFER_SIZE);
            int count = 0;
            int match = 1;

            while(*(pInput + count) != NULL) {
                if(*(pInput + count) != *(pQuit + count)) {
                    match = 0;
                    break;
                }
                count++;
            }

            if(match) {
                exit_code = EXIT_SUCCESS;
                break;
            }

            argc = split(argv, MAX_ARGUMENTS, pInput);

            int index = 0;
            while(index < argc)
            {
                char *arg = do_getarg(index);
                if(arg == NULL) {
                    do_print("Invalid get arguments index. Exitt !!!\n");
                    exit(ERROR_INVALID_INDEX);
                }
                if(exec_file(arg))
                    do_print("exec_file failed\n");
                index++;
            }
        }
    }
    else {
        exit_code = ERROR_NULL_POINTER;
    }

    exit(exit_code);
}

