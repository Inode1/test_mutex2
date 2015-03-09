/*
 * test_ipc.h
 *
 *  Created on: Mar 5, 2015
 *      Author: ivan
 */

#ifndef TEST_IPC_H_
#define TEST_IPC_H_

#include <semaphore.h>
#include <pthread.h>

#define SHMNAME    "/MRSTPSharedMemory"
#define SEMEVENT   "/MRSTPEventSemaphore"
#define SEMNSTORED "/cond_empty"
#define SEMNEMPTY  "/MRSTPNEmptySemaphore"


struct semaphore {
    pthread_mutex_t lock;
    pthread_cond_t nonzero;
    unsigned count;
};
typedef struct semaphore semaphore_t;
typedef struct msg_struct_
{
    unsigned int           data_len;
    // next data go (raw memory)
} msg_struct;

typedef struct control_struct_
{
	pthread_mutex_t  mutex_var;
	pthread_cond_t   cond_var;
	unsigned int     msg_size;
    char             msg_count;
    char             msg_status;
    char             read_state;
    char             write_state;
} control_struct;

typedef struct msq_init_
{
    control_struct   msg_data;
    void            *shm_start;
    pthread_cond_t  *shm_cond;
    size_t           shm_size;
    sem_t           *sem_nstored;
    sem_t           *sem_event;
    sem_t           *sem_nempty;
} msq_init;

// if error return -1, msg_count - count message and
// msq_size max length message
int msq_initialization(msq_init *msq, ushort msg_count, unsigned int msg_size);

void msq_deinitialization(msq_init *msq);
// if message can`t send return -1, length message must be
// less then size message in msq_init.
int msq_send(msq_init *msq, const char *message, size_t message_len);

#endif /* TEST_IPC_H_ */
