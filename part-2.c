/*
 * file:        part-2.c
 * description: Part 2, CS5600 load-and-switch assignment, Fall 2020
 */

#include "elf64.h"
#include "sysdefs.h"

/* Definitions */

#define EOF    (-1)    // standard value for end of file
#define STDIN_FILE_DESCRIPTOR_NUMBER    0    // standard value for input file descriptor 
#define STDOUT_FILE_DESCRIPTOR_NUMBER    1    // standard value for output file descriptor 
#define STDERROR_FILE_DESCRIPTOR_NUMBER    2    // standard value for error file descriptor 
#define MAX_BUFFER_SIZE    200    // maximum size to do read and write
#define MAX_ARGC    10    // max number of input arguments 
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
    #define SEEK_SET    0    /* set file offset to offset */
#endif
#ifndef SEEK_CUR
    #define SEEK_CUR    1    /* set file offset to current plus offset */
#endif
#ifndef SEEK_END
    #define SEEK_END    2    /* set file offset to EOF plus offset */
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

#define ROUND_UP(a,b)    (((a+b-1)/b)*b)    /* round A up to the next multiple of B */

#define M_OFFSET    0x8000000    /* Micro program offset */

/* Global functions */
extern void *vector[];

/* Global variables */

char *argv[MAX_ARGC];    /* Global variable to store command arguments. */

int argc;    /* Global variable to store maximum number of command arguments. */

typedef struct {    /* Struct to hold mmap mapped memory addresses */
    void *addr;
    int len;
} memory_t;

/* Function declarations*/

/*
 * Function to read a line from stdin (file descriptor 0) into a buffer.
 *
 * @param fd: file descriptor value in int
 * @param ptr: pointer to the buffer
 * @param len: length to read
 * @return length of the characters read in integer
 */
int read(int fd, void *ptr, int len);

/*
 * Function to print a string to stdout (file descriptor 1) from a buffer.
 *
 * @param fd: file descriptor value in int
 * @param ptr: pointer to the buffer
 * @param len: length to write
 * @return length of the characters written in integer
 */
int write(int fd, void *ptr, int len);

/*
 * Function to exit the program.
 *
 * @param err: error value on exit in int
 */
void exit(int err);

/*
 * Function to read a complete line from stdin.
 *
 * @param pInput: pointer to the buffer in char
 * @param len: length to read
 * @return length of the characters read in integer
 */
int readline(char *pInput, int len);

/*
 * Function to write a complete line to stdout.
 *
 * @param pInput: pointer to the buffer in char
 */
void print(char *pInput);

int open(char *path, int flags);
int close(int fd);
int lseek(int fd, int offset, int flag);
void *mmap(void *addr, int len, int prot, int flags, int fd, int offset);
int munmap(void *addr, int len);
void do_readline(char *buf, int len);
void do_print(char *buf);
char *do_getarg(int i); 
int compare_string(char *str1, char*str2, int length);
void run_program(int fd);
void load_program(int fd, int offset, memory_t *mapped_addrs, int *loaded_len);
void exec_program(void *entry, int offset);
void remove_mapping(memory_t *mapped_addrs, int mapped_len);

/* function definitions */

int read(int fd, void *ptr, int len)
{
    int ret = FUNCTION_FAILURE;
    int readLength = 0;

    void *curr = ptr;
    // with length of 0 returns zero and has no other effects
    if (len > 0) {
        // Read input character one at a time until the given length.
        do {
            syscall(__NR_read, fd, curr++, 1);
            ret = readLength++;
        } while (readLength < len);
    } else {
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
    int readLength = 0;
    char *cPtr = pInput;
    char c;
    // with length of 0 returns zero and has no other effects
    if (len > 0) {
        // Read input character one at a time until the given length.
        do {
            syscall(__NR_read, STDIN_FILE_DESCRIPTOR_NUMBER, &c, 1);
            cPtr[readLength++] = c;
        } while (c != '\n' && c != EOF && readLength < len);
        cPtr[readLength] = '\0';  // NULL terminate
        ret = readLength;
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


void load_program(int fd, int offset, memory_t *mapped_addrs, int *loaded_len) {
    /* Read the main header (offset 0) */
    struct elf64_ehdr hdr;
    lseek(fd, 0, SEEK_SET);
    read(fd, &hdr, sizeof(hdr));
    int total_len = hdr.e_phnum;

    /* read program headers (offset 'hdr.e_phoff') */
    struct elf64_phdr phdrs[total_len];
    lseek(fd, hdr.e_phoff, SEEK_SET);
    read(fd, phdrs, sizeof(phdrs));

    void *start_addr = (void *)0x0 + offset;

    int i;
    for (i = 0; i < total_len; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            int mem_needed_sz = phdrs[i].p_memsz;
            int mem_rounded_sz = ROUND_UP(mem_needed_sz, 4096);

            void *buf = mmap(start_addr, mem_rounded_sz,
                             PROT_READ | PROT_WRITE | PROT_EXEC,
                             MAP_PRIVATE | MAP_ANONYMOUS, fd, 0);
            if (buf == MAP_FAILED) {
                print("mmap failed\n");
                remove_mapping(mapped_addrs, *loaded_len);
                exit(EXIT_FAILURE);
            }

            memory_t memory = {.addr = buf, .len = mem_rounded_sz};
            mapped_addrs[*loaded_len] = memory;
            (*loaded_len)++;
            start_addr += mem_rounded_sz;

            lseek(fd, (int)phdrs[i].p_offset, SEEK_SET);
            read(fd, buf, mem_needed_sz);
        }
    }
}

void exec_program(void *entry, int offset) {
    void (*f)();
    f = entry + offset;
    f();
}

void remove_mapping(memory_t *mapped_addrs, int mapped_len) {
    int i;
    for (i = 0; i < mapped_len; i++) {
        memory_t mem = mapped_addrs[i];
        munmap(mem.addr, mem.len);
    }
}

void run_program(int fd) {
    /* Get total entry numbers for mapped address allocation. */
    struct elf64_ehdr hdr;
    lseek(fd, 0, SEEK_SET);
    read(fd, &hdr, sizeof(hdr));
    int total_len = hdr.e_phnum;

    /* Hold mmap mapped address for future removal. */
    memory_t mapped_addrs[total_len];

    int loaded_len = 0;

    load_program(fd, M_OFFSET, mapped_addrs, &loaded_len);

    exec_program(hdr.e_entry, M_OFFSET);

    remove_mapping(mapped_addrs, loaded_len);
}

int compare_string(char *str1, char*str2, int length)
{
    int count = 0;
    int match = 1;

    while((*(str1 + count) != NULL) && (count < length)) {
        if(*(str1 + count) != *(str2 + count)) {
            match = 0;
            break;
        }
        count++;
    }
    return match;
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

    while(1) {
        do_print("> ");
        readline(pInput, MAX_BUFFER_SIZE);

        if(compare_string(pInput, pQuit, 5)) {
            exit_code = EXIT_SUCCESS;
            break;
        }

        argc = split(argv, MAX_ARGC, pInput);

        int index = 0;
        char *arg = do_getarg(index);
        if(arg == NULL) {
            do_print("Invalid get arguments index. continue !!!\n");
            continue;
        }

        int fd = open(arg, O_RDONLY);
        if (fd < 0) {
            do_print("Selected micro program doesn't exist\n");
            continue;
        }

        run_program(fd);
        close(fd);
    }

    exit(exit_code);
}
