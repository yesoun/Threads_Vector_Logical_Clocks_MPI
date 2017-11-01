/**
   Author: Yassine Maalej (Github: yesoun) 
   email: maalej.yessine@gmail.com && maal4948@vandals.uidaho.edu
   Date: March 2016
   Class: CS541 Advanced Operating Systems
   Insitution: University Of Idaho

*/

/**
For this assignment, you are going to implement a vector logical clocks experiment. Each node will send messages to the other nodes; the messages will include the node’s view of “time”
as it knows it. The receiving node will then update its view of time at the other nodes from the information received in the message. We noted that one node’s notion of time at the other
nodes improves when more messages are sent. This assignment will explore that notion. For this assignment, each node - represented by a separate thread - will send messages to other
random nodes (i.e., the destination node will be selected randomly). The message will consist of the sending node’s vector clock values. The receiving node will update its vector with the
information that has been received. In order to see the effect of “busy” nodes (those sending a lot of messages) vs “non-busy” nodes, the times at which nodes send messages should be
random, but based on a distribution. One way to do this is to vary the sending times of each node, determined by its rank. On average, the root node should send out one message per
unit time, the next node should send out 1/2 per unit time, the next node should send out 1/4 per unit time, etc. That is, the number of messages a node sends should be related to its rank n as
1/2^n.
Your program should create at least 4 threads. Run the experiment long enough so that several hundred messages are sent, then compare the vector clocks at each node. Comment on the
trend that you see in the accuracy of the clocks at each node. You implemented a message-passing mechanism for pthreads in assignment 1 - you should use that mechanism to pass the messages among
threads for this assignment. You may need to extend that capability for this assignment, to handle messages sent to multiple threads.
*/

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctime>
#include <list>
#include <vector>
#include <cstdlib>   /** std::srand() and std::rand() */
#include <algorithm> /** std::unique, std::distance */
#include <cmath> /** POW */
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <cassert>
#include <unistd.h>
#include <stdlib.h>


#define MY_MQ_NAME "/my_mq"
#define data_mess_length 30
using namespace std;


/** Total Number of Threads */
const int numberOfThreads = 4;

/** Definition of Threads that represents Nodes, 4 in minimum */
pthread_t thread1;
pthread_t thread2;
pthread_t thread3;
pthread_t thread4;



/** vector clock that contains the lamport clock of each thread, 4 in total */  
struct lamport_vector_clock
{
int a;
int b;
int c;
int d;
};

/** structs_messages_queues  */
static struct mq_attr my_message_queue;
static mqd_t my_mq;

/** enumerate the data types */ 
enum {
    DATA_TYPE_U32,
    DATA_TYPE_I32,
    DATA_TYPE_U16,
    DATA_TYPE_I16,
    DATA_TYPE_U8,
    DATA_TYPE_F32,
    DATA_TYPE_STR
};

/** data structure of the message exchaged */
typedef union message_data_structure {
    unsigned int data_u32;
    unsigned short int data_u16;
    unsigned char data_u8;
    unsigned char array[data_mess_length];
    int data_i32;
    short int data_i16;
    float data_f32;
} my_data_t;


/** struct exchanged message */ 
struct exchan_Message
{
	int thread_id;
	struct lamport_vector_clock vck;
};
typedef struct my_mq_msg_s {
    int type;
    my_data_t data;
} my_mq_msg_t;

/** main function of threads */ 
void* mainThread1(void*);
void* mainThread2(void*);
void* mainThread3(void*);
void* mainThread4(void*);

/** Check the received signal **/
void signal_hanlder(int signum) {
    if (signum != SIGINT) {
        printf("Received invalid signum = %d in signal_hanlder()\n", signum);
        assert(signum == SIGINT);
    }

    printf("Received SIGINT. Exiting Application\n");

    pthread_cancel(thread1);
    pthread_cancel(thread2);

    mq_close(my_mq);
    mq_unlink(MY_MQ_NAME);

    exit(0);
}

/** Thread writes changes its vector clock before it is being sent ot other threads (similarly considered as nodes)*/
/** Used to send the message containing the vector logical clocks from pth_i to pth_j */
/** Used to read the Vector Clock from the sent thread and update the current vector clock */
/** Define the distribution function that randomly picks random nodes that are going to receive the vector clock of the thread that has called it */

/** main thread 1 */
void* mainThread1(void*) {
    unsigned int exec_period_usecs;
    int status;
    my_mq_msg_t send_msg;
    int cnt=0;

    exec_period_usecs = 1000000; /*in micro-seconds*/

    printf("Thread1 start  time to execute = %d uSecs\n",exec_period_usecs);
    while(1) {
        switch (cnt) {

            case 0:
                /* Send a U32 type value on the MQ */
                send_msg.type = DATA_TYPE_U32;
                send_msg.data.data_u32 = 99999999;
                break;
            case 1:
                /* Send an I32 type value on the MQ */
                send_msg.type = DATA_TYPE_I32;
                send_msg.data.data_i32 = -1324287;
                break;
            case 2:
                /* Send a U16 type value on the MQ */
                send_msg.type = DATA_TYPE_U16;
                send_msg.data.data_u16 = 100;
                break;
            case 3:
                /* Send a I16 type value on the MQ */
                send_msg.type = DATA_TYPE_I16;
                send_msg.data.data_i16 = -45;
                break;
            case 4:
                /* Send a U8 type value on the MQ */
                send_msg.type = DATA_TYPE_U8;
                send_msg.data.data_u8 = 5;
                break;
            case 5:
                /* Send a F32 type value on the MQ */
                send_msg.type = DATA_TYPE_F32;
                send_msg.data.data_f32 = 3.1415;
                break;
            case 6:
                /* Send a STR type value on the MQ */
                send_msg.type = DATA_TYPE_STR;
                //sprintf(send_msg.data.array,data_mess_length,"This is a test str\n");
                break;
            default:
                printf("Invalid counter value in sending thread\n");
                break;
        }

        status = mq_send(my_mq, (const char*)&send_msg, sizeof(send_msg), 1);
        assert(status != -1);

        cnt += 1;

        if (cnt > 6) {
           cnt = 0;
        }
        usleep(exec_period_usecs);
    }
}


/** main thread 2*/
void* mainThread2(void*) {
    unsigned int exec_period_usecs;
    int status;
    my_mq_msg_t recv_msg;
    exec_period_usecs = 10000; /*in micro-seconds*/
    printf("Thread2 start  time to execute = %d uSecs\n",exec_period_usecs);
    while(1) {
        status = mq_receive(my_mq, (char*)&recv_msg,sizeof(recv_msg), NULL);

        if (status == sizeof(recv_msg)) {

            switch(recv_msg.type) {
                case DATA_TYPE_U32:
                    printf("Received data type:   U32,   value = %u\n",recv_msg.data.data_u32);
                    break;
                case DATA_TYPE_I32:
                    printf("Received data type:   I32,   value = %d\n",recv_msg.data.data_i32);
                    break;
                case DATA_TYPE_U16:
                    printf("Received data type:   U16,   value = %hu\n",recv_msg.data.data_u16);
                    break;
                case DATA_TYPE_I16:
                    printf("Received data type:   I16,   value = %hd\n",recv_msg.data.data_i16);
                    break;
                case DATA_TYPE_U8:
                    printf("Received data type:   U8,    value = %hhu\n",recv_msg.data.data_u8);
                    break;
                case DATA_TYPE_F32:
                    printf("Received data type:   F32,   value = %f\n",recv_msg.data.data_f32);
                    break;
                case DATA_TYPE_STR:
                    printf("Received data type:   STR,   value = %s\n",recv_msg.data.array);
                    break;
                default:
                    printf("Received invalid data type\n");
                    assert(0);
                    break;
            }
        }

        usleep(exec_period_usecs);
    }
}

/** main thread 3 */
void* mainThread3(void*) {
    unsigned int exec_period_usecs;
    int status;
    my_mq_msg_t send_msg;
    int cnt=0;

    exec_period_usecs = 1000000; /*in micro-seconds*/

    printf("Thread2 start  time to execute = %d uSecs\n",exec_period_usecs);
    while(1) {
        switch (cnt) {

            case 0:
                /* Send a U32 type value on the MQ */
                send_msg.type = DATA_TYPE_U32;
                send_msg.data.data_u32 = 99999999;
                break;
            case 1:
                /* Send an I32 type value on the MQ */
                send_msg.type = DATA_TYPE_I32;
                send_msg.data.data_i32 = -1324287;
                break;
            case 2:
                /* Send a U16 type value on the MQ */
                send_msg.type = DATA_TYPE_U16;
                send_msg.data.data_u16 = 100;
                break;
            case 3:
                /* Send a I16 type value on the MQ */
                send_msg.type = DATA_TYPE_I16;
                send_msg.data.data_i16 = -45;
                break;
            case 4:
                /* Send a U8 type value on the MQ */
                send_msg.type = DATA_TYPE_U8;
                send_msg.data.data_u8 = 5;
                break;
            case 5:
                /* Send a F32 type value on the MQ */
                send_msg.type = DATA_TYPE_F32;
                send_msg.data.data_f32 = 3.1415;
                break;
            case 6:
                /* Send a STR type value on the MQ */
                send_msg.type = DATA_TYPE_STR;
                //snprintf(send_msg.data.array,data_mess_length,"This is a test str\n");
                break;
            default:
                printf("Invalid counter value in sending thread\n");
                break;
        }

        status = mq_send(my_mq, (const char*)&send_msg, sizeof(send_msg), 1);
        assert(status != -1);

        cnt += 1;

        if (cnt > 6) {
           cnt = 0;
        }
        usleep(exec_period_usecs);
    }
}


/** main thread 4 */
void* mainThread4(void*) {
    unsigned int exec_period_usecs;
    int status;
    my_mq_msg_t send_msg;
    int cnt=0;

    exec_period_usecs = 1000000; /*in micro-seconds*/

    printf("Thread4 start  time to execute = %d uSecs\n",exec_period_usecs);
    while(1) {
        switch (cnt) {

            case 0:
                /* Send a U32 type value on the MQ */
                send_msg.type = DATA_TYPE_U32;
                send_msg.data.data_u32 = 99999999;
                break;
            case 1:
                /* Send an I32 type value on the MQ */
                send_msg.type = DATA_TYPE_I32;
                send_msg.data.data_i32 = -1324287;
                break;
            case 2:
                /* Send a U16 type value on the MQ */
                send_msg.type = DATA_TYPE_U16;
                send_msg.data.data_u16 = 100;
                break;
            case 3:
                /* Send a I16 type value on the MQ */
                send_msg.type = DATA_TYPE_I16;
                send_msg.data.data_i16 = -45;
                break;
            case 4:
                /* Send a U8 type value on the MQ */
                send_msg.type = DATA_TYPE_U8;
                send_msg.data.data_u8 = 5;
                break;
            case 5:
                /* Send a F32 type value on the MQ */
                send_msg.type = DATA_TYPE_F32;
                send_msg.data.data_f32 = 3.1415;
                break;
            case 6:
                /* Send a STR type value on the MQ */
                send_msg.type = DATA_TYPE_STR;
                //snprintf(send_msg.data.array,data_mess_length,"This is a test str\n");
                break;
            default:
                printf("Invalid counter value in sending thread\n");
                break;
        }

        status = mq_send(my_mq, (const char*)&send_msg, sizeof(send_msg), 1);
        assert(status != -1);

        cnt += 1;

        if (cnt > 6) {
           cnt = 0;
        }
        usleep(exec_period_usecs);
    }
}



/**
    Compilation : g++ -Wall hmwrk_2_main.c -o exec_hmwrk_2_main --lpthread -lrt
    Main
*/
int main(void) {
    pthread_attr_t attr;
    int status; 

    signal(SIGINT, signal_hanlder);

    my_message_queue.mq_maxmsg = 10;
    my_message_queue.mq_msgsize = sizeof(my_mq_msg_t);

    my_mq = mq_open(MY_MQ_NAME, \
                    O_CREAT | O_RDWR | O_NONBLOCK, \
                    0666, \
                    &my_message_queue);

    assert(my_mq != -1);

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1024*1024);

    printf("First Create Thread1\n");
    status = pthread_create(&thread1, &attr, mainThread1, NULL);
    if (status != 0) {
        printf("Failed to create thread1 with status = %d\n", status);
        assert(status == 0);
    }

    printf("First Create Thread2\n");
    status = pthread_create(&thread2, &attr, mainThread2, NULL);
    if (status != 0) {
        printf("Failed to create thread2 with status = %d\n", status);
        assert(status == 0);
    }
     printf("First Create Thread3\n");
    status = pthread_create(&thread2, &attr, mainThread3, NULL);
    if (status != 0) {
        printf("Failed to create thread3 with status = %d\n", status);
        assert(status == 0);
    }
     printf("First Create Thread4\n");
    status = pthread_create(&thread2, &attr, mainThread4, NULL);
    if (status != 0) {
        printf("Failed to create thread4 with status = %d\n", status);
        assert(status == 0);
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    signal_hanlder(SIGINT);

    return 0;
}

