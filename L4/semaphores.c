/*
Modify your producer-consumer implementation so that it uses 
semaphores to handle race conditions instead of mutexes.
Use the pthread library implementation.
To test your implementation you can try using 2 producers and 1 consumer.
*/

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 20

int count = 0;
int buffer [BUFFER_SIZE];
int in = 0;
int out = 0;

pthread_t tid;
pthread_mutex_t mutexlock;

sem_t empty, full;


void insert(int item)
{
   pthread_mutex_lock(&mutexlock);
   while (count == BUFFER_SIZE); // wait until it is full
   buffer[in] = item;
   printf("in: %d inserted: %d and count is %d\n", in, item, count);
   in = (in +1) % BUFFER_SIZE;
   count++;
   pthread_mutex_unlock(&mutexlock);
   sleep(1); 
}


int remove_item()
{
   int item;
   pthread_mutex_lock(&mutexlock);
   while (count == 0); // wait until it is empty
   item = buffer[out];
   printf("out: %d removed: %d and count is: %d\n", out, item, count);
   out = (out + 1)%BUFFER_SIZE;
   count--;
   pthread_mutex_unlock(&mutexlock);
   sleep(1); 
   return item;
}


void * producer(void * param)
{
   int item;
   while(1)
   {
      sem_wait(&empty);
      item = rand()%BUFFER_SIZE ;
      insert(item);
      sem_post(&full);
   }
}


void * consumer(void * param){
   int item;
   while(1)
   {
        sem_wait(&full);
   	    item = remove_item();
        sem_post(&empty);
   }
}


int main(int argc, char * argv[])
{
    int producers = atoi(argv[1]);
    int consumers =  atoi(argv[2]);
   //  int producers = 1; /* For testing */
   //  int consumers = 2; /* For testing */
    int i;

    pthread_mutex_init(&mutexlock, NULL);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);

    for (i = 0; i < producers; i++)
       pthread_create(&tid, NULL, producer,NULL);

    for (i = 0; i < consumers; i++)
       pthread_create(&tid, NULL, consumer, NULL); 

    pthread_join(tid,NULL);
}