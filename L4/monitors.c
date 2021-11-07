// #include <pthread.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>

// #define BUFFER_SIZE 20

// int in = 0, out = 0;
// int buffer [BUFFER_SIZE];

// pthread_mutex_t mutex;
// pthread_cond_t full, empty;

// void insert(int item){
//    pthread_mutex_lock(&mutex);
//    if ((in + 1) % BUFFER_SIZE == out) {
// 	pthread_cond_signal(&empty);
// 	pthread_cond_wait(&full, &mutex);	
//    }
//    buffer[in] = item;
//    in = (in + 1) % BUFFER_SIZE;
//    sleep(1);
//    pthread_mutex_unlock(&mutex);
// }

// int remove_item(){
//    int item;
//    pthread_mutex_lock(&mutex);
//    if (in == out) {
// 	pthread_cond_signal(&full);
// 	pthread_cond_wait(&empty, &mutex);
//    }
//    item = buffer[out];
//    out = (out + 1) % BUFFER_SIZE;
//    sleep(1);
//    pthread_mutex_unlock(&mutex);
//    return item;
// }

// void * producer(void * param){
//    int item;
//    while(1){
//       item = rand() % BUFFER_SIZE ;
//       insert(item);
//       printf("in: %d inserted: %d\n", in, item);
//    }
// }

// void * consumer(void * param){
//    int item;
//    while(1){
//    	item = remove_item();
//    	printf("out: %d removed: %d\n", out, item);
//    }
// }

// int main(int argc, char * argv[])
// {
//     pthread_mutex_init(&mutex, NULL);

//     // int producers = atoi(argv[1]);
//     // int consumers = atoi(argv[2]);
//     int producers = 1; /* For testing */
//     int consumers = 2; /* For testing */
//     pthread_t tid_producers[producers];
//     pthread_t tid_consumers[consumers];
    
//     int i;

//     for (i = 0; i < producers; i++)
//        pthread_create(&tid_producers[i], NULL, producer,NULL);

//     for (i = 0; i < consumers; i++)
//        pthread_create(&tid_consumers[i], NULL, consumer, NULL); 

//     for (i = 0; i < producers; i++)
//        pthread_join(tid_producers[i],NULL);

//     for (i = 0; i < consumers; i++)
//        pthread_join(tid_consumers[i],NULL);

//     pthread_mutex_destroy(&mutex);

// }


#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>


#define BUFFER_SIZE 20

int count = 0;

int in = 0;

int out = 0;

int buffer[BUFFER_SIZE];

pthread_t tid;

pthread_mutex_t lock;

pthread_mutex_t cond_mutex;

pthread_cond_t cond;

void insert(int item) {

  pthread_mutex_lock( & lock);

  while (count == BUFFER_SIZE);

  buffer[ in ] = item;

  count++;

  sleep(1);

  pthread_mutex_unlock( & lock);

}

int remove_item() 
{
   pthread_mutex_lock(&lock);

   int item;
   while (count == 0);
   item = buffer[out];
   count--;
   sleep(1);

   pthread_mutex_unlock( & lock);

   return item;
}

void * producer(void * param) {


  pthread_mutex_lock( & cond_mutex);

   int item;
   while (1) 
   {
      pthread_cond_wait( & cond, & cond_mutex);

      if (count < BUFFER_SIZE) 
      {
      item = rand() % BUFFER_SIZE;
      insert(item);
      in = ( in +1) % BUFFER_SIZE;
      printf("in: %d inserted: %d\n", in , item);
      }
  }

  pthread_mutex_unlock( & cond_mutex);

}

void * consumer(void * param) {


   pthread_mutex_lock( & cond_mutex);
   int item;
   while (1) 
   {
      pthread_cond_signal( & cond);
      if (count > 0) 
      {
      item = remove_item();
      out = (out + 1) % BUFFER_SIZE;
      printf("out: %d removed: %d\n", out, item);
      }

      pthread_mutex_unlock( & cond_mutex);
   }

}

int main(int argc, char * argv[]) {

//   int producers = 2; 
  int producers = atoi(argv[1]);

//   int consumers = 1; 
  int consumers = atoi(argv[2]);

  int i;

  for (i = 0; i < producers; i++)

    pthread_create( & tid, NULL, producer, NULL);

  for (i = 0; i < consumers; i++)

    pthread_create( & tid, NULL, consumer, NULL);

  pthread_join(tid, NULL);

  pthread_mutex_destroy( & lock);

}