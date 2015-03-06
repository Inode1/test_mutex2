/*
 * test_ipc.c
 *
 *  Created on: Mar 2, 2015
 *      Author: ivan
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#include "test_ipc.h"

int main()
{
    msq_init msq;
    if (msq_initialization(&msq, 50, 100) == -1)
    {
        perror("msq_init");
    }
    int i = 0;
    for (; i < 150; ++i)
    {
        char s[50];
        sprintf(s,"Hello :%d", i);

        msq_send(&msq, s, strlen(s));
    }

    msq_deinitialization(&msq);
    return 0;
}


int msq_initialization(msq_init *msq, ushort msg_count, unsigned int msg_size)
{
    int shm;
    char *data;
    int page_size;
    int size_raw_data;
    int power;
    int exist = 0;

    memset((void *)msq, 0, sizeof(msq_init));
    msq->msg_data.msg_count = msg_count;
    msq->msg_data.msg_size  = msg_size;
    if ((shm = shm_open(SHMNAME, O_CREAT|O_RDWR | O_EXCL, 0644)) == -1 )
    {
        if (errno != EEXIST)
        {
            perror(SHMNAME " shm_open");
            return -1;
        }
        exist = 1;
    }
    if (exist)
    {
        if ((shm = shm_open(SHMNAME, O_RDWR, 0644)) == -1)
        {
            perror(SHMNAME " shm_open");
            return -1;
        }
    }
    if ((msq->sem_nempty = sem_open(SEMNEMPTY, O_CREAT, 0644, 0)) ==  SEM_FAILED)
    {
        perror(SEMNEMPTY " sem_open");
        return -1;
    }
    *(char *)msq->sem_nempty = msq->msg_data.msg_count;

    if ((msq->sem_nstored = sem_open(SEMNSTORED, O_CREAT, 0644, 0)) ==  SEM_FAILED)
    {
        perror(SEMNSTORED " sem_open");
        return -1;
    }
    *(char *)msq->sem_nstored = 0;
    if ((msq->sem_event = sem_open(SEMEVENT, O_CREAT, 0644, 0)) ==  SEM_FAILED)
    {
        perror(SEMEVENT " sem_open");
        return -1;
    }
    *(char *)msq->sem_event = 0;

    page_size = getpagesize();
    size_raw_data = msg_count * (msg_size + sizeof(msg_struct)) + sizeof(control_struct);
    power = size_raw_data / page_size;
    if (size_raw_data % page_size)
    {
        ++power;
    }

    msq->shm_size = power * page_size;

/*    if (!exist)
    {*/
        if (ftruncate(shm, msq->shm_size) == -1)
        {
            perror("ftruncate");
            return -1;
        }


    data = mmap(0, msq->shm_size, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    if ( data == (char*)-1 )
    {
        perror("mmap");
        return -1;
    }
    msq->shm_start = data;

    // now place control structure in memory
    if (exist)
    {
        msq->msg_data.msg_status = ((control_struct *)msq->shm_start)->msg_status;
    }
    memcpy(msq->shm_start, &msq->msg_data, sizeof(control_struct));
    return 0;
}

void msq_deinitialization(msq_init *msq)
{
    // destroy shared memory
    if (munmap(msq->shm_start, msq->shm_size) == -1)
    {
        perror("shmdt");
    }

/*    if (shm_unlink(SHMNAME) == -1)
    {
        perror("shm_unlink");
    }*/
    //destroy semaphore_event
/*    if (sem_unlink(SEMEVENT) == -1)
    {
        perror(SEMEVENT" destroy");
    }*/
    //destroy semaphore_nstored
/*    if (sem_unlink(SEMNSTORED) == -1)
    {
        perror(SEMNSTORED" destroy");
    }*/
}

int msq_send(msq_init *msq, const char *message, size_t message_len)
{
    if (((control_struct *)msq->shm_start)->msg_status)
    {
        ((control_struct *)msq->shm_start)->read_state = 0;
        ((control_struct *)msq->shm_start)->write_state = 0;
        ((control_struct *)msq->shm_start)->msg_status = 0;
        *(char *)msq->sem_nstored = 0;
        *(char *)msq->sem_nempty = msq->msg_data.msg_count;
        sem_post(msq->sem_event);
    }


    if (msq->shm_start == (void *)0 || message_len > msq->shm_size)
    {
        return -1;
    }


    if (sem_trywait(msq->sem_nempty))
    {
        return -1;
    }
    printf("In lock\n");

    sleep(1);

    control_struct *point = ((control_struct *)msq->shm_start);

    point->write_state %= point->msg_count;
    size_t offset_from_begin= (point->write_state * (point->msg_size + sizeof(msg_struct))) + sizeof(control_struct);

    msg_struct *msg_point = (msg_struct *)((char *) point + offset_from_begin);

    msg_point->data_len = message_len;
    printf("Message :%s", message);
    printf("Message len :%d", message_len);
    char *data = (char *)(&msg_point->data_len + 1);
    memcpy( data, message, message_len);
    ++point->write_state;
    sem_post(msq->sem_nstored);
    printf("In unlock\n");
    return 0;
}
