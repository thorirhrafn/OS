#include "message_queue.h"
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/*
 * The commands supported by the server
 */
typedef enum _Command {
    CmdExit      = 0x00,     // Stops the server

    CmdAdd,                  // Adds the two message parameters
    CmdSubtract              // Subtracts the two message parameters
} Command;

/*
 * The message format to be sent to the server.
 */
typedef struct _Message {
    /*
     * One of the command constants.
     */
    Command command;
    /*
     * Used as operand 1 (if required)
     */
    int parameter1;
    /*
     * Used as operand 2 (if required)
     */
    int parameter2;
} Message;

#define QUEUE_NAME "/simple_calculator"

#define FORMAT_STRING_ADD      "%d + %d = %d\n"
#define FORMAT_STRING_SUBTRACT "%d - %d = %d\n"

#define QUEUE_PERMISSIONS 0660

mqd_t startClient(void)
{
    // TODO: Open the message queue previously created by the server

    int queid = mq_open(QUEUE_NAME, O_WRONLY);

    return queid;
}

int sendExitTask(mqd_t client)
{

    Message input;
    input.command = CmdExit;
    input.parameter1 = 0;
    input.parameter2 = 0;

    int id = mq_send(client, (char*)&input, sizeof(input), 0);


    // TODO: Send the exit command to the server.
    return id;
}

int sendAddTask(mqd_t client, int operand1, int operand2)
{
    // TODO: Send the add command with the operands

    Message input;
    input.command = CmdAdd;
    input.parameter1 = operand1;
    input.parameter2 = operand2;

    int id = mq_send(client, (char*)&input, sizeof(input), 0);

    return id;
}

int sendSubtractTask(mqd_t client, int operand1, int operand2)
{
    Message input;
    input.command = CmdSubtract;
    input.parameter1 = operand1;
    input.parameter2 = operand2;

    int id = mq_send(client, (char*)&input, sizeof(input), 0);


    // TODO: Send the sub command with the operands
    return id;
}


int stopClient(mqd_t client)
{
//    (void)client;

    // TODO: Clean up anything on the client-side

    int id = mq_close(client);

    return id;

}

int runServer(void)
{
    int didExit = 0;
    int hadError;
    Message msg;

    struct mq_attr attr;
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = 10;           // Maximum number of messages in the queue
    attr.mq_msgsize = sizeof(msg);  // Maximum message size
    attr.mq_curmsgs = 0;

    // TODO:
    // Create and open the message queue. Server only needs to read from it.
    // Clients only need to write to it, allow for all users.

    mqd_t server = 0;
    
    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0400, &attr);

    printf("mq: %d\n", mq);

    if (mq==-1) {
        return -1;
    }
    
    // This is the implementation of the server part, already completed:
    // TODO: You may have to make minor extensions in order to satisfy all requirements
    do {
        // Attempt to receive a message from the queue.
        
        ssize_t received = mq_receive(server, (char*)&msg, sizeof(msg), NULL);
        
        if (received != sizeof(msg)) {
            // This implicitly also checks for error (i.e., -1)
            hadError = 1;
            didExit=1;
            continue;
        }
        printf("140\n");
        switch (msg.command)
        {
            case CmdExit:
                printf("144\n");
                // End this loop.
                didExit = 1;
                break;

            case CmdAdd:
                printf("150\n");
                // Print the required output.
                printf(FORMAT_STRING_ADD,
                       msg.parameter1,
                       msg.parameter2,
                       msg.parameter1 + msg.parameter2);
                break;

            case CmdSubtract:
                printf("159\n");
                // Print the required output.
                printf(FORMAT_STRING_SUBTRACT,
                       msg.parameter1,
                       msg.parameter2,
                       msg.parameter1 - msg.parameter2);
                break;

            default:
                printf("168\n");
                didExit=1;
                break;
        }
    } while (!didExit);

    // TODO
    // Close the message queue on exit and unlink it

    mq_close(mq);
	mq_unlink(QUEUE_NAME);

    //stopClient(server);
    if (hadError == 1) {
        return -1;
    }

    return 0;
}


//====== TEST 5: You call mq_open in runServer correctly. =====
//Testing message_queue.c with test5.c
//Starting to test message_queue.c...
//x:a:expect
//x:c:expect
//x:d:expect
//x:e:expect
//Starting to open server 
//x:a:here
//125
//130
//x:a:expected
//Label c not reached.
//x:c:expected
//Label d not reached.
//x:d:expected
//Label e not reached.
//x:e:expected
//You have errors in your solution, please fix them.
//x:error
//TEST FAILED




// ====== TEST 6: Your server exits when receiving an exit command. =====
//Testing message_queue.c with test6.c
//Starting to test message_queue.c...
//x:a:expect
//Starting to open server 
//125
//130
//x:a:here
//139
//msg operand 1 : -1433787952
//msg operand 2 : 32609
//cmd exit
//x:a:expected
//All test cases passed.
//x:success
// ====== TEST 7: You close and unlink the queue correctly. =====
//Testing message_queue.c with test7.c
//Starting to test message_queue.c...
//x:b:here
//x:a:expect
//x:b:expect
//Starting to open server 
//125
//130
//139
//msg operand 1 : 0
//msg operand 2 : 0
//cmd exit
//x:a:here
//x:a:expected
//Label b not reached.
//x:b:expected
//x:b:here
//You have errors in your solution, please fix them.
//x:error
//TEST FAILED
// ====== TEST 8: 0.5: Your server gives the right output. =====
//Testing message_queue.c with test8.c
//Starting to test message_queue.c...
//runServer: Bad file descriptor
//Running test client
//startClient: Permission denied
//TEST FAILED