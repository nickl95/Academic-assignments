/*
  Nicholas LaFramboise
  fj1000
  CSC 4421: At-Home Lab #2
*/

/*

Run instructions:
Compile with: "gcc -pthread Lab2_NL.c"
Once compiled, run: "./a.out arg1 arg2 arg2"
Arguments are integers (time, # of producers, # of consumers)

*/

typedef int buffer_item;
#define BUFFER_SIZE 5

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t mutex; // Declaring the mutex lock

sem_t full, empty; // Declaring the semaphore

buffer_item buffer[BUFFER_SIZE]; // Declaring the buffer

int count; // Declaring buffer counter

pthread_t tid; // Declaring the thread ID
pthread_attr_t attr; // Declaring thread attributes

void *producer(void *param); // Declaring the producer thread
void *consumer(void *param); // Declaring the consumer thread

void *producer(void *param)
{
  buffer_item buff;
  while (1)
  {
    /* Producing an item in the next_produced */
    int ranNum = rand() / 100000000; // Sleep for a random time

    sleep(ranNum);

    buff = rand(); // Randomising the buffer

    sem_wait(&empty); // Acquiring empty lock
    pthread_mutex_lock(&mutex); // Acquire MUTEX lock

    /* Adding the produced item to the buffer */

    // If true, error. insert_item should be returning 0
    if (insert_item(buff)) { fprintf(stderr, "Unable to create producer\n"); }
    else { printf("Producer produced %d\n", buff); }

    pthread_mutex_unlock(&mutex); // Release MUTEX lock
    sem_post(&full); // Signal full
  }
}

void *consumer(void *param) // Consumer is basically the same as Producer, just an inverted job
{
  buffer_item buff;

  while (1)
  {
    int ranNum = rand() / 100000000;
    sleep(ranNum);

    sem_wait(&full); // Signal full
    pthread_mutex_lock(&mutex);

    /* Removing the produced item from the buffer */

    // Compiler recommended fprintf over printf
    if(remove_item(&buff)) { fprintf(stderr, "Unable to create consumer\n"); } 
    else { printf("Consumer consumed %d\n", buff); }

    pthread_mutex_unlock(&mutex);
    sem_post(&empty); // Signal empty

  }
}

int insert_item(buffer_item buff) // Insert item into buffer
{
  // If the buffer isn't full, add an item and increment the count
  if (count < BUFFER_SIZE)
  {
    buffer[count] = buff;
    count++;
    return 0;
  }
  // If buffer is full...
  else { return -1; }
}

int remove_item(buffer_item *buff) // Removing item from buffer (about the same as insert_item, inverted)
{
  if (count > 0)
  {
    *buff = buffer[(count-1)];
    count--;
    return 0;
  }

  else { return -1; }
}

int main(int argc, char *argv[])
{
  int i; // Used in the for loops
  int mainSleepTime = atoi(argv[1]); // Sleep timer
  int proNum = atoi(argv[2]); // Number of producers
  int conNum = atoi(argv[3]); // Number of consumers

  pthread_mutex_init(&mutex, NULL);
  sem_init(&full, 0, 0);
  sem_init(&empty, 0, BUFFER_SIZE);
  pthread_attr_init(&attr); // Moved this from main to here, different from last lab
  count = 0;

  // Create the producer threads
  for(i = 0; i < proNum; i++) { pthread_create(&tid,&attr,producer,NULL); }

  // Create the consumer threads
  for(i = 0; i < conNum; i++) { pthread_create(&tid,&attr,consumer,NULL); }

  sleep(mainSleepTime); // Good night

  return 0;
}
