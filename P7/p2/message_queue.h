#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <mqueue.h>

mqd_t startClient(void);

int sendExitTask(mqd_t client);

int sendAddTask(mqd_t client, int operand1, int operand2);

int sendSubtractTask(mqd_t client, int operand1, int operand2);

int stopClient(mqd_t client);

int runServer(void);

#endif