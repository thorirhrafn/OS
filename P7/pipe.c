// This changes the way some includes behave.
// This should stay before any include.
//magni21 thorirhh21
#define _GNU_SOURCE

#include "pipe.h"
#include <sys/wait.h> /* For waitpid */
#include <unistd.h> /* For fork, pipe */
#include <stdlib.h> /* For exit */
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#define READ_END 0
#define WRITE_END 1

int run_program(char *file_path, char *argv[])
{
    if ((file_path == NULL) || (argv == NULL)) 
    {
        return -1;
    }

    // -------------------------
    // TODO: Open a pipe
    // -------------------------

    int pipe_fd[2];
    pipe2(pipe_fd, O_CLOEXEC);

    int child_pid = fork();
    if (child_pid == -1) 
    {
        return -1;
    } 
    else if (child_pid == 0) 
    {

        // -------------------------
        // TODO: Write the error on the pipe
        // -------------------------

        close(pipe_fd[READ_END]);
        
        execvp(file_path, argv);

        int x = errno;

        write(pipe_fd[WRITE_END], &x, sizeof(x));
        
        close(pipe_fd[WRITE_END]);   
        
        exit(0);
    } 
    else 
    {
        int status, hadError = 0;

        close(pipe_fd[WRITE_END]);

        int waitError = waitpid(child_pid, &status, 0);
        if (waitError == -1) 
        {
            // Error while waiting for child.
            close(pipe_fd[READ_END]);
            hadError = 1;
        } 
        else if (!WIFEXITED(status)) 
        {

            // Our child exited with another problem (e.g., a segmentation fault)
            // We use the error code ECANCELED to signal this.

            close(pipe_fd[READ_END]);
            hadError = 1;
            errno = ECANCELED;

        } 
        else 
        {
            
            // -------------------------
            // TODO: If there was an execvp error in the child, set errno
            //       to the value execvp set it to.
            // -------------------------
            
            int x = 0;

            int readId = read(pipe_fd[READ_END], &x, sizeof(x));

            errno = x;

            close(pipe_fd[READ_END]);
            
            if (readId != 0) {   //execvp unsuccessful
                return -1;
            }
            
            return readId;
        }
        return hadError ? -1 : WEXITSTATUS(status);
    }
}



//====== TEST 5: 0.5: You return -1 on execvp errors. =====
//Testing pipe.c with test5.c
//Starting to test pipe.c...
//read status: -1
//Read statusCode 0
//Errno 9
///home/sty22/A7p1/.tests/test5.c line 16: result => Expected 0, but got -1
//Write status code: -1
//read status: -1
//Read statusCode 0
//Errno 9
//Write status code: -1
//read status: -1
//Read statusCode 0
//Errno 9
//You have errors in your solution, please fix them.
//x:error
//TEST FAILED

// ====== TEST 6: You set the right errno on execvp errors. =====
//Testing pipe.c with test6.c
//Starting to test pipe.c...
//read status: -1
//Read statusCode 0
//Errno 9
///home/sty22/A7p1/.tests/test6.c line 16: result => Expected 0, but got -1
///home/sty22/A7p1/.tests/test6.c line 17: errno => Expected 0, but got 9
//Write status code: -1
//read status: -1
//Read statusCode 0
//Errno 9
///home/sty22/A7p1/.tests/test6.c line 26: errno => Expected 2, but got 9
//Write status code: -1
//read status: -1
//Read statusCode 0
//Errno 9
///home/sty22/A7p1/.tests/test6.c line 35: errno => Expected 13, but got 9
//You have errors in your solution, please fix them.
//x:error
//TEST FAILED
//