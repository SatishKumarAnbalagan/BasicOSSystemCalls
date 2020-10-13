/*
 * file:        part-3.c
 * description: part 3, CS5600 load-and-switch assignment, Fall 2020
 */

#include "elf64.h"
#include "sysdefs.h"

/* Definitions */

#define EOF    (-1)    /* standard value for end of file */
#define STDIN_FILE_DESCRIPTOR_NUMBER    0    /* standard value for input file descriptor */
#define STDOUT_FILE_DESCRIPTOR_NUMBER    1    /* standard value for output file descriptor */
#define STDERROR_FILE_DESCRIPTOR_NUMBER    2    /* standard value for error file descriptor */
#define MAX_BUFFER_SIZE    200    /* maximum size to do read and write */
#define MAX_ARGC    10    /* max number of input arguments */
#define PAGE_SIZE    4096    /* size of the virtual page */

/* Open file mode flags definitions */
#ifndef O_RDONLY
    #define O_RDONLY    00000000    /* read-only */
#endif
#ifndef O_WRONLY
    #define O_WRONLY    00000001    /* write-only */
#endif
#ifndef O_RDWR
    #define O_RDWR      00000002    /* read/write */
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


extern void *vector[];
extern void switch_to(void **location_for_old_sp, void *new_value);
extern void *setup_stack0(void *_stack, void *func);

/* Function declarations*/

// System Call wrappers

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
 * Function to open a file.
 * 
 * @param path: pointer to a valid address of file in char
 * @param flags: operation permission type in int
 *               O_RDONLY - read-only
 *               O_WRONLY - write-only
 *               O_RDWR - read/write
 * @return valid file descriptor in int
 */
int open(char *path, int flags);

/*
 * Function to close a file.
 * 
 * @param fd: valid file descriptor in int
 * @return returns zero on success, else -1
 */
int close(int fd);

/*
 * Function repositions the offset of the open file associated with the file descriptor according to flag.
 * 
 * @param fd: valid file descriptor in int
 * @param offset: reposition offset value in int
 * @param flag: SEEK_SET - The offset is set to offset bytes
 *              SEEK_CUR - The offset is set to its current location plus offset bytes
 *              SEEK_END - The offset is set to the size of the file plus offset bytes
 * @return returns the resulting offset location in bytes from the beginning of the file, else -1
 */
int lseek(int fd, int offset, int flag);

/*
 * Function asks to map len bytes starting at offset offset from the file (or other object) specified by
 * the file descriptor fd into memory, preferably at address addr.
 * 
 * @param addr: memory address
 * @param len: length of mapping bytes in int
 * @param prot: PROT_EXEC - Pages may be executed
 *              PROT_READ - Pages may be read
 *              PROT_WRITE - Pages may be written
 * @param flags: specifies the type of the mapped object, mapping options and whether modifications
 *               made to the mapped copy of the page are private to the process or
 *               are to be shared with other references.
 *               MAP_PRIVATE - Create a private copy-on-write mapping
 *               MAP_ANONYMOUS - mapping is not backed by any file; the fd and offset arguments are ignored.
 * @param fd: valid file descriptor in int
 * @param offset: offset value in int
 * @return the actual place where the object is mapped is returned by mmap()
 */
void *mmap(void *addr, int len, int prot, int flags, int fd, int offset);

/*
 * Function deletes the mappings for the specified address range.
 * 
 * @param addr: memory address
 * @param len: length of mapping bytes in int 
 * @return on success returns 0, on failure -1
 */
int munmap(void *addr, int len);

// utility functions

/*
 * Function to write a buffer to stdout.
 * 
 * @param buf: pointer to the buffer in char
 */
void print(char *buf);

/*
 * Utility function to write a buffer to stdout.
 * 
 * @param buf: pointer to the buffer in char
 */
void do_print(char *buf);

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
    if (len > 0) {
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
    if (path != NULL)
    {
        ret = syscall(__NR_open, path, flags);
    }
    // checks fd validity & returns file descriptor
    return FD_VALID_CHECK(ret) ? ret : FUNCTION_FAILURE;
}

int close(int fd)
{
    int ret = FUNCTION_FAILURE;
    if (FD_VALID_CHECK(fd))
    {
        ret = syscall(__NR_close, fd);
    }
    return ret;
}

int lseek(int fd, int offset, int flag)
{
    int ret = FUNCTION_FAILURE;
    if (FD_VALID_CHECK(fd))
    {
        ret = syscall(__NR_lseek, fd, offset, flag);
    }
    return ret; 
}

void *mmap(void *addr, int len, int prot, int flags, int fd, int offset)
{
    void* ret = (void*) FUNCTION_FAILURE;
    if ((len > 0))
    {
        ret = (void*) syscall(__NR_mmap, addr, len, prot, flags, fd, offset); 
        if (ret == MAP_FAILED)
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
    if (len > 0) {
        ret = syscall(__NR_munmap, addr, len);
    }
    return ret;
}

/* copy from Part 2 */
void do_print(char *buf);

/* ---------- */

/* write these new functions */
void do_yield12(void);
void do_yield21(void);
void do_uexit(void);

/* ---------- */

void main(void)
{
    vector[1] = do_print;

    vector[3] = do_yield12;
    vector[4] = do_yield21;
    vector[5] = do_uexit;
    
    /* your code here */

    do_print("done\n");
    exit(0);
}
