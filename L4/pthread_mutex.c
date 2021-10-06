#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 20
int count = 0;
int buffer [BUFFER_SIZE];

pthread_t tid;
pthread_mutex_t lock;

void insert(int item){
    pthread_mutex_lock(&lock);
    while (count == BUFFER_SIZE);
    buffer[count] = item;
    count++;
    sleep(1); 
    pthread_mutex_release(&lock);
}
int remove_item(){
    int item;
    pthread_mutex_lock(&lock);
    while (count == 0);
    item = buffer[count];
    count--;
    sleep(1); 
    pthread_mutex_release(&lock);
    return item;

}
void * producer(void * param){
   int item;
   while(1){
      item = rand() % BUFFER_SIZE ;
      insert(item);
      printf("inserted: %d\n", item);
   }
}
void * consumer(void * param){
   int item;
   while(1){
   	item = remove_item();
   	printf("removed: %d\n", item);
   }
}
int main(int argc, char * argv[])
{
    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    int i;
    for (i = 0; i < producers; i++)
       pthread_create(&tid, NULL, producer,NULL);
    for (i = 0; i < consumers; i++)
       pthread_create(&tid, NULL, consumer, NULL); 
    pthread_join(tid,NULL);
    pthread_mutex_destoy(&lock);
}