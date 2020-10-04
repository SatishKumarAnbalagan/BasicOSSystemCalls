/*
 * file:        part-1.c
 * description: Part 1, CS5600 load-and-switch assignment, Fall 2020
 */

/* THE ONLY INCLUDE FILE */
#include "sysdefs.h"

#define EOF (-1)    // standard value for end of file
#define STDIN_FILE_DESCRIPTOR_NUMBER 0    // standard value for input file descriptor 
#define STDOUT_FILE_DESCRIPTOR_NUMBER 1    // standard value for output file descriptor 
#define STDERROR_FILE_DESCRIPTOR_NUMBER 2    // standard value for error file descriptor 
#define MAX_BUFFER_SIZE 200    // maximum size to do read and write

/* write these functions */

int read(int fd, void *ptr, int len);
int write(int fd, void *ptr, int len);
void exit(int err);
void readline(char *pInput);
void print(void *pInput);

/* read one line from stdin (file descriptor 0) into a buffer: */
void readline(char *pInput)
{
    int i = 0;
    unsigned int length = 0;
    char c;
    // Read input character one at a time until EOF and newline character is found.
    do {
        length += read(STDIN_FILE_DESCRIPTOR_NUMBER, &c, 1);
        pInput[i++] = c;
    } while (c != '\n' && c != EOF);
    pInput[i] = '\0';  //NULL terminate
}

void print(void *pInput)
{
    unsigned int length = 0;
    char *ptr = (char*) pInput;
    char c = *((char*) pInput);
    // Read input characters until EOF and newline character is found.
    while (c != NULL) {
        length += write(STDOUT_FILE_DESCRIPTOR_NUMBER, &c, 1);
        c = *(++ptr);
    }
}

int read(int fd, void *ptr, int len)
{
    return syscall(__NR_read, fd, ptr, len);
}

/* print a string to stdout (file descriptor 1) */
int write(int fd, void *ptr, int len)
{
    return syscall(__NR_write, fd, ptr, len);
}

void exit(int err)
{
    syscall(__NR_exit, err);
}

void main(void)
{
    char input[MAX_BUFFER_SIZE] = {0};
    char *pInput = &input[0];
    char quit[6] = {'q', 'u', 'i', 't', '\n', '\0'};
    char *pQuit = &quit[0];

    while(1) {
        print("> ");
        readline(pInput);
        int count = 0;
        int match = 1;
        while(count < 6) {
            if(*(pInput+count) != *(pQuit+count)) {
                match = 0;
                break;
            }
            count++;
        }
        if(match)
            break;

        print("you typed: ");
        print(pInput);
    }
    exit(0);
}
