#include "scheduler.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct _QueueItem {
    /*
     * The data of this item.
     */
    int data;
    /*
     * The next item in the queue.
     * NULL if there is no next item.
     */
    struct _QueueItem *next;
} QueueItem;

typedef struct _Queue {
    /*
     * The first item of the queue.
     * NULL if the queue is empty.
     */
    QueueItem *head;
    /*
     * The last item of the queue.
     * undefined if the queue is empty.
     */
    QueueItem *tail;
} Queue;

typedef enum _ThreadState {
    STATE_UNUSED = 0, // This entry in the _threads array is unused.

    STATE_READY,      // The thread is ready and should be on a ready queue for
                      //   selection by the scheduler
    STATE_RUNNING,    // The thread is running and should not be on a ready queue
    STATE_WAITING     // The thread is blocked and should not be on a ready queue
} ThreadState;

typedef struct _Thread {
    int threadId;
    ThreadState state;
    /*
     * Range: 0 ... HIGHEST_PRIORITY (including)
     * HIGHEST_PRIORITY is highest priority
     */
    int priority;
} Thread;

Thread _threads[MAX_THREADS] = {{0}};

/* TODO: Add global variables if needed. */
Queue readyQueue;
Queue waitingQueue; 
Queue runQueue;
int currentPriority;
int starvationCounter[HIGHEST_PRIORITY+1];

//make arrays to handle queues of different priority
Queue readyBlock[HIGHEST_PRIORITY+1] = {{0}};
//Queue runningBlock[HIGHEST_PRIORITY+1];


/*
 * Adds a new, waiting thread.
 */
int startThread(int threadId, int priority)
{
    if (((threadId < 0) || (threadId >= MAX_THREADS) ||
        (_threads[threadId].state != STATE_UNUSED)) ||
        (priority < 0) || (priority > HIGHEST_PRIORITY)) {

        return -1;
    }

    _threads[threadId].threadId = threadId;
    _threads[threadId].state    = STATE_WAITING;
    _threads[threadId].priority = priority;
    return 0;
}


void initQueue(Queue *queue)
{
    queue->head = NULL;
    queue->tail = NULL;
}
/*
 * Append to the tail of the queue.
 * Does nothing on error.
 */
void _enqueue(Queue *queue, int data)
{
    (void)queue;
    (void)data;

    //printf("Setja id: %d i queue \n", data);

    // Create a new queue item to insert into the queue
    QueueItem *newQueueItem = malloc(sizeof(QueueItem));
    newQueueItem->data = data;
    newQueueItem->next = NULL;

    //if the qeue has a tail, link the new item to the tail
    if (queue->tail != NULL)
    {
        queue->tail->next = newQueueItem;
    }
    //set the new item as the new tail
    queue->tail = newQueueItem;
    //if the queue doesn't have a head then the tail is also it's head
    if (queue->head == NULL)
    {
        //printf("setja id: %d sem head \n", data);
        queue->head = newQueueItem;
    }
}

/*
 * Remove and get the head of the queue.
 * Return -1 if the queue is empty.
 */
int _dequeue(Queue *queue)
{
    (void)queue;
    int queueItemData;

    //Start by checking if the queue is empty
    if (queue->head == NULL)
    {
        //queue is empty, return -1
        return -1;
    }
    else
    {
        //get the dequeued item and fetch it's data
        QueueItem *dequeuedItem = queue->head;
        queueItemData = dequeuedItem->data;
        //point the head of the queue to the next item to make a new head
        queue->head = queue->head->next;
        //if this was the last item in the queue set the tail also to NULL
        if (queue->head == NULL)
        {
            queue->tail = NULL;
        }

        //printf("queueItemData = %d \n", queueItemData);
        //free the alocated memory for the dequeued item
        free(dequeuedItem);

        //return the data from the dequeued item
        return queueItemData; 
    }
}

void initScheduler()
{
    // TODO: Implement if you need to initialize any global variables you added
    //initialize ready and running arrays
    printf("Byrja ad testa \n");
    printf("HIGHEST_PRIORITY: %d \n", HIGHEST_PRIORITY);
    //int max = (int)HIGHEST_PRIORITY+1;
    //printf("max: %d \n", max);
    currentPriority = 0;  

    for (int n = 0; n < HIGHEST_PRIORITY+1; n++)
    {
        //printf("%d \n", n);
        //printf("gera queue i array \n");
        Queue ready;
        initQueue(&ready);
        readyBlock[n] = ready;
        starvationCounter[n] = 0;
        /*
        Queue running;
        initQueue(&running);
        runningBlock[n] = running;
        */

        //printf("queue komid í array \n");
    }

    /*
    initQueue(&readyQueue);
    initQueue(&waitingQueue);
    initQueue(&runQueue);
    
    Queue testQ;
    initQueue(&testQ);

    printf("Setja i queue \n");
    _enqueue(&testQ, 1);
    _enqueue(&testQ, 2);
    _enqueue(&testQ, 3);

    int n;
    while((n = _dequeue(&testQ)) > 0)
    {
        printf("n = %d \n", n);
    }
    */
}

/*
 * Called whenever a waiting thread gets ready to run.
 */
void onThreadReady(int threadId)
{
    (void)threadId;
    // TODO: Implement
    //printf("on thread ready, thread id: %d \n", threadId);

    if (threadId >= 0 && threadId < MAX_THREADS)
    {
        //printf("Thread id legal, do enqueue \n");
        //printf("Thread status: %d \n", _threads[threadId].state);
        if (_threads[threadId].state == STATE_WAITING)
        {
            _threads[threadId].state = STATE_READY;
            int priority = _threads[threadId].priority;
            if (priority > currentPriority) { currentPriority = priority; }
            //Queue queue = readyBlock[priority];
            //printf("Thread ready: do enqueue for id: %d , priority: %d \n", threadId, priority);
            _enqueue(&readyBlock[priority], threadId);
        }
    }
    else
    {
        printf("Thread id not legal, cancel enqueue \n");
    }
}

/*
 * Called whenever a running thread is forced of the CPU
 * (e.g., through a timer interrupt).
 */
void onThreadPreempted(int threadId)
{
    (void)threadId;
    // TODO: Implement
    if (threadId >= 0 && threadId < MAX_THREADS)
    {
        //printf("on thread preempted, thread id: %d \n", threadId);
        if (_threads[threadId].state == STATE_RUNNING)
        {
            _threads[threadId].state = STATE_READY;
            int priority = _threads[threadId].priority;
            if (priority > currentPriority) { currentPriority = priority; }

            //Queue queue = readyBlock[priority];
            //printf("Thread preempted: do enqueue for id: %d , priority: %d \n" , threadId, priority);
            _enqueue(&readyBlock[priority], threadId);
        }
    }
}

/*
 * Called whenever a running thread needs to wait.
 */
void onThreadWaiting(int threadId)
{
    (void)threadId;
    // TODO: Implement
    if (threadId >= 0&& threadId < MAX_THREADS)
    {
        //printf("on thread preempted, thread id: %d \n", threadId);
        if (_threads[threadId].state == STATE_RUNNING)
        {
            _threads[threadId].state = STATE_WAITING;
        }
    }

}

/*
 * Gets the id of the next thread to run and sets its state to running.
 */
int scheduleNextThread()
{
    // TODO: Implement
    printf("Scheduling .... \n");
    printf("Current priority: %d \n", currentPriority);

    starvationCounter[currentPriority] += 1;

    printf("Starvation ticktets: %d , for priority: %d \n", starvationCounter[currentPriority], currentPriority);

    if (starvationCounter[currentPriority] > 5)
    {
        for (int i = currentPriority-1; i >= 0; i--)
        {
            if (i >= 0)
            {
                printf("i: %d \n", i);
                if (readyBlock[i].head != NULL)
                {
                    starvationCounter[currentPriority] =0;
                    int tempPriority = currentPriority;
                    currentPriority = i;
                    int id = _dequeue(&readyBlock[currentPriority]);

                    //printf("Scheduling for id: %d \n", id);
                    currentPriority = tempPriority;
        
                    if (id >= 0 && id < MAX_THREADS)
                    {
                        _threads[id].state = STATE_RUNNING;
                        return id;
                    }
                    else
                    {
                        return -1;
                    }

                    break;
                }
            }
        }
    }
    /*
    printf("Skoda inn i queue array: \n");
    for (int i = 0; i < 6; i++)
    {
        if (readyBlock[i].head != NULL)
        {
            printf("Head data: %d i queue nr. %d \n", i, readyBlock[i].head->data);
        }
        else
        {
            printf("Queue nr. %d er tomt \n", i);
        }
    }
    */

    //Queue queue = readyBlock[currentPriority];

    /*
    if (queue.head != NULL)
    {
        printf("Data in queue: %d \n", queue.head->data);
    }
    else
    {
        printf("queue virdist vera tomt \n");
    }
    */
    int id = _dequeue(&readyBlock[currentPriority]);

    //printf("Scheduling for id: %d \n", id);
    
    if (id >= 0 && id < MAX_THREADS)
    {
        _threads[id].state = STATE_RUNNING;
        return id;
    }
    else
    {
        return -1;
    }
}