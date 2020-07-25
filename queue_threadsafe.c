#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

//needed to allow building and running

#define MESSAGE_LENGTH 100

typedef struct sMessage{
    char content[MESSAGE_LENGTH];
} Message;

bool QueueMutex; //true means available

bool mutex_get(bool mutex)
{
    return mutex;
}

void mutex_release(bool mutex)
{
}

//design this type:
typedef struct sQueueElement{
    Message* message;
    struct sQueueElement* next;
 } QueueElement;

typedef struct sQueue {
    QueueElement * first;
    QueueElement * last;
} Queue;


// design and implement these functions:

/**

  * This function blocks waiting for a message on the

  * specified queue. When a message is added to the queue,

  * this function returns a pointer to the message.

  */

Message* getNextMessage(Queue *p_queue)
{
    if (p_queue->last == NULL)
        return NULL;

    while (!mutex_get(QueueMutex));
    
    QueueElement * last_queue_item = p_queue->last;
    
    p_queue->last = p_queue->last->next;
    
    if (p_queue->last == NULL) //if queue is now empty
        p_queue->first = NULL;
    
    Message* message_copy = last_queue_item->message;

    free(last_queue_item);

    mutex_release(QueueMutex);

    return message_copy;
}

/**

 * This function adds a message to the specified queue. If

 * successful returns true, otherwise false. (Assume we have a

 * type bool with values true and false.)

 */

bool addMessage(Queue *p_queue, Message *p_message)
{
    if(!mutex_get(QueueMutex))
        return false;
    
    QueueElement * newElement;
    newElement = malloc(sizeof(QueueElement));
    
    newElement->message = malloc(sizeof(Message));
    memcpy(newElement->message, p_message, MESSAGE_LENGTH*sizeof(char));
    
    newElement->next = NULL;
    
    if(p_queue->first == NULL) //if queue is empty
    {
        p_queue->first = newElement;
        p_queue->last = newElement;
        return true;
    }
    
    p_queue->first->next = newElement;
    p_queue->first = newElement;
    
    mutex_release(QueueMutex);
    return true;
}



int main()
{
    printf("Queue Demo \n\n");
    
    Queue * messageQueue;
    messageQueue = malloc(sizeof(Queue));
    messageQueue->last = NULL;
    messageQueue->first = NULL;
    
    Message newMessage = {"This is a message"};
    
    //1
    printf("#1: Adding a message\n");
    
    QueueMutex = true;
    if (addMessage(messageQueue, &newMessage))
    {
        printf("Addition of a new message OK\n");
        printf("New queue element content: `%s` \n", messageQueue->last->message->content);
    }
    else
        printf("Addition of a new message FAILED\n");
    
    //2
    printf("\n#2: Adding multiple messages and counting queue size\n");
    
    strcpy(newMessage.content, "The message content is now different");
    addMessage(messageQueue, &newMessage);
    addMessage(messageQueue, &newMessage);
    addMessage(messageQueue, &newMessage);
    addMessage(messageQueue, &newMessage);
    
    QueueMutex = false; //these additions will fail
    addMessage(messageQueue, &newMessage);
    addMessage(messageQueue, &newMessage);
    
    int size = 0;
    QueueElement * currentElement = messageQueue->last;
    
    while(currentElement!=NULL)
    {
        size++;
        currentElement = currentElement->next;
    }
    printf("The Queue size is %d \n", size);
    assert(size==5);
    
    //3
    printf("\n#3: Getting messages while owning the mutex \n");
    QueueMutex = true;
    Message * receivedMessage;
    
    getNextMessage(messageQueue);
    receivedMessage = getNextMessage(messageQueue);

    printf("Last received message content: `%s` \n", receivedMessage->content);
    
    currentElement = messageQueue->last;
    size = 0;
    while(currentElement!=NULL)
    {
        size++;
        currentElement = currentElement->next;
    }
    printf("The Queue size is %d \n", size);
    assert(size==3);
    
    //4
    printf("\n#4: Trying to get a message while NOT owning the mutex (waits for mutex release)\n");
    QueueMutex = false;
    receivedMessage = getNextMessage(messageQueue);

    printf("Last received message content: `%s` \n", receivedMessage->content);
    
    currentElement = messageQueue->last;
    size = 0;
    while(currentElement!=NULL)
    {
        size++;
        currentElement = currentElement->next;
    }
    printf("The Queue size is %d \n", size);
    assert(size==2);
}

