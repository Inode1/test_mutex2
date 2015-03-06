/*
 * test_ipc.h
 *
 *  Created on: Mar 5, 2015
 *      Author: ivan
 */

#ifndef TEST_IPC_H_
#define TEST_IPC_H_

#define SHMNAME    "/MRSTPSharedMemory"
#define SEMEVENT   "/MRSTPEventSemaphore"
#define SEMNSTORED "/MRSTPNStoredSemaphore"
#define SEMNEMPTY  "/MRSTPNEmptySemaphore"

typedef struct msg_struct_
{
    unsigned int           data_len;
    // next data go (raw memory)
} msg_struct;

typedef struct control_struct_
{
    unsigned int   msg_size;
    char           msg_count;
    char           msg_status;
    char           read_state;
    char           write_state;
} control_struct;

typedef struct msq_init_
{
    control_struct   msg_data;
    void            *shm_start;
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
