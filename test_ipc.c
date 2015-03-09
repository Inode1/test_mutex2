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
#include <errno.h>

#include "test_ipc.h"

int main()
{
    msq_init msq;
    printf("All good : %d",sizeof(pthread_cond_t));
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
    int shm, shm_cond;
    char *data;
    int page_size;
    int size_raw_data;
    int power;
    int exist = 0;
    control_struct my;

    printf("All good\n");








/*    pthread_condattr_t attrcond;

     Initialise attribute to condition.
    pthread_condattr_init(&attrcond);
    pthread_condattr_setpshared(&attrcond, PTHREAD_PROCESS_SHARED);*/
    //pthread_cond_init(&cond, &attrcond);
    printf("All good\n");
    memset((void *)msq, 0, sizeof(msq_init));
    memset((void *)&my, 0, sizeof(control_struct));


    int res;
    pthread_cond_t m_cond;
       pthread_condattr_t cond_attr;
       res = pthread_condattr_init(&cond_attr);
       if(res != 0){
           perror("cond");
           return -1;
       }
       res = pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
       if(res != 0){
    	   pthread_condattr_destroy(&cond_attr);
           perror("cond");
           return -1;
       }
       res = pthread_cond_init(&my.cond_var, &cond_attr);
       pthread_condattr_destroy(&cond_attr);
       if(res != 0){
           perror("cond");
           return -1;
       }

       pthread_mutex_t m_mutex;
          pthread_mutexattr_t mutex_attr;
          res = pthread_mutexattr_init(&mutex_attr);
          if(res != 0){
              perror("cond");
              return -1;
          }
          res = pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
          if(res != 0){
       	   pthread_mutexattr_destroy(&mutex_attr);
              perror("cond");
              return -1;
          }
          res = pthread_mutex_init(&my.mutex_var, &mutex_attr);
          pthread_mutexattr_destroy(&mutex_attr);
          if(res != 0){
              perror("cond");
              return -1;
          }

    my.msg_count = msg_count;
    my.msg_size  = msg_size;
    if ((shm = shm_open(SHMNAME, O_CREAT|O_RDWR | O_EXCL, 0666)) == -1 )
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
/*    if ((msq->sem_nempty = sem_open(SEMNEMPTY, O_CREAT, 0644, 0)) ==  SEM_FAILED)
    {
        perror(SEMNEMPTY " sem_open");
        return -1;
    }
    *(char *)msq->sem_nempty = 1; */
    if ((shm_cond = shm_open(SEMNSTORED, O_CREAT|O_RDWR|O_EXCL, 0666)) ==  -1)
    {
        perror(SEMNSTORED " sem_open");
        return -1;
    }
    printf("All good\n");

   /* if ((msq->sem_nstored = sem_open(SEMNSTORED, O_CREAT, 0644, 0)) ==  SEM_FAILED)
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
    *(char *)msq->sem_event = 0;*/

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


        printf("All good\n");
    // now place control structure in memory
    if (exist)
    {
        msq->msg_data.msg_status = ((control_struct *)msq->shm_start)->msg_status;
    }
/*    memcpy(&msq->msg_data.mutex_var, &m_mutex, sizeof(pthread_mutex_t));
    memcpy(&msq->msg_data.cond_var, &m_cond, sizeof(pthread_cond_t));*/
    memcpy(msq->shm_start, &my, sizeof(control_struct));
    pthread_mutex_lock(&((control_struct*)msq->shm_start)->mutex_var);
    printf("Control struct ready\n");
    sleep(2);
    pthread_mutex_unlock(&((control_struct*)msq->shm_start)->mutex_var);
/*    pthread_cond_destroy(&m_condition);
    pthread_mutex_destroy(&m_mutex);*/
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
/*    if (((control_struct *)msq->shm_start)->msg_status)
    {
        ((control_struct *)msq->shm_start)->read_state = 0;
        ((control_struct *)msq->shm_start)->write_state = 0;
        ((control_struct *)msq->shm_start)->msg_status = 0;typedef struct semaphore semaphore_t;
        *(char *)msq->sem_nstored = 0;
        *(char *)msq->sem_nempty = msq->msg_data.msg_count;
        sem_post(msq->sem_event);
    }*/


    if (msq->shm_start == (void *)0 || message_len > msq->shm_size)
    {
        return -1;
    }
    sleep(2);
    control_struct *point = ((control_struct *)msq->shm_start);
pthread_mutex_lock(&point->mutex_var);


    printf("%c  In lock\n", *(char *)&msq->msg_data.mutex_var);

    sleep(2);



    point->write_state %= point->msg_count;
    size_t offset_from_begin= (point->write_state * (point->msg_size + sizeof(msg_struct))) + sizeof(control_struct);

    msg_struct *msg_point = (msg_struct *)((char *) point + offset_from_begin);

    msg_point->data_len = message_len;
    char *data = (char *)(&msg_point->data_len + 1);
    memcpy( data, message, message_len);
    ++point->write_state;
    point->msg_status = 1;


    pthread_cond_signal(&point->cond_var);
    pthread_mutex_unlock(&point->mutex_var);

    //pthread_cond_signal(msq->shm_cond);
    printf("Out unlock\n");
    return 0;
}


semaphore_t *
semaphore_create(char *semaphore_name)
{
int fd;
    semaphore_t *semap;
    pthread_mutexattr_t psharedm;
    pthread_condattr_t psharedc;

    fd = open(semaphore_name, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (fd < 0)
        return (NULL);
    (void) ftruncate(fd, sizeof(semaphore_t));
    (void) pthread_mutexattr_init(&psharedm);
    (void) pthread_mutexattr_setpshared(&psharedm,
        PTHREAD_PROCESS_SHARED);
    (void) pthread_condattr_init(&psharedc);
    (void) pthread_condattr_setpshared(&psharedc,
        PTHREAD_PROCESS_SHARED);
    semap = (semaphore_t *) mmap(NULL, sizeof(semaphore_t),
            PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, 0);
    close (fd);
    (void) pthread_mutex_init(&semap->lock, &psharedm);
    (void) pthread_cond_init(&semap->nonzero, &psharedc);
    semap->count = 0;
    return (semap);
}
