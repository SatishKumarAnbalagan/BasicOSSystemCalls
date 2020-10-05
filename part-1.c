/*
 * file:        part-1.c
 * description: Part 1, CS5600 load-and-switch assignment, Fall 2020
 * 
 * Author - Satish Kumar Anbalagan
 * Date - Oct 03, 2020
 * Email ID - anbalagan.s@northeastern.edu
 */

/* THE ONLY INCLUDE FILE */
#include "sysdefs.h"

/* Definitions */
#define EOF (-1)    // standard value for end of file
#define STDIN_FILE_DESCRIPTOR_NUMBER 0    // standard value for input file descriptor 
#define STDOUT_FILE_DESCRIPTOR_NUMBER 1    // standard value for output file descriptor 
#define STDERROR_FILE_DESCRIPTOR_NUMBER 2    // standard value for error file descriptor 
#define MAX_BUFFER_SIZE 200    // maximum size to do read and write

/* Error code definitions */
#define ERROR_NULL_POINTER (-1000)

/* Exit code definitions */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

/* function exit code definitions */
#define FUNCTION_SUCCESS (EXIT_SUCCESS)
#define FUNCTION_FAILURE (EXIT_FAILURE)

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
 */
void readline(char *pInput);

/*
 * Function to write a complete line to stdout.
 * 
 * @param pInput: pointer to the buffer in char
 */
void print(void *pInput);

/* Function Definitions */

void readline(char *pInput)
{
    int i = 0;
    unsigned int length = 0;
    char c;

    // Read input character one at a time until EOF and newline character is found with NULL termination.
    do {
        length += read(STDIN_FILE_DESCRIPTOR_NUMBER, &c, 1);
        pInput[i++] = c;
    } while (c != '\n' && c != EOF);
    pInput[i] = '\0';  //NULL terminate
}

void print(void *pInput)
{
    unsigned int length = 0;
    char *ptr;
    char c;
    
    ptr = (char*) pInput;
    c = *((char*) pInput);
    // Read input characters until EOF and newline character is found.
    while (c != NULL) {
        length += write(STDOUT_FILE_DESCRIPTOR_NUMBER, &c, 1);
        c = *(++ptr);
    }
}

int read(int fd, void *ptr, int len)
{
    unsigned int ret = FUNCTION_FAILURE;
    unsigned int readLength = 0;
    char *cPtr = (char*) ptr;
    char c;

    // with length of 0 returns zero and has no other effects
    if(! len) {
        ret = 0;
    }

    // Read input character one at a time until the given length.
    while (c != '\n' && c != EOF && readLength < len) {
        syscall(__NR_read, fd, &c, 1);
        cPtr[readLength++] = c;
        ret = readLength;
    }
    return ret;
}

int write(int fd, void *ptr, int len)
{
    return syscall(__NR_write, fd, ptr, len);
}

void exit(int err)
{
    syscall(__NR_exit, err);
}

/* Main function */
void main(void)
{
    char input[MAX_BUFFER_SIZE] = {0};
    char *pInput = &input[0];
    char quit[5] = {'q', 'u', 'i', 't', '\n'};
    char *pQuit = &quit[0];
    int exit_code = EXIT_FAILURE;

    if(pInput != NULL) {    // NULL check, if in case malloc is used in future
        while(1) {
            print("> ");
            readline(pInput);
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

            print("you typed: ");
            print(pInput);
        }
    }
    else {
        exit_code = ERROR_NULL_POINTER;
    }

    exit(exit_code);
}
