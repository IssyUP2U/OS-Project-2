//Isabella Sobey
//ims190003
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define NO_TELLERS 3
#define NO_CUSTOMERS 50

int main( ) {
    srand(time(NULL));
    int r = rand() % 10; //random number between 0 and 9

    //initialize semaphoresssss
    sem_init(&door, 0, 2); //2 threads can enter the door
    sem_init(&manager, 0, 1); //1 thread can access the manager at a time
    sem_init(&safe, 0, 2); //teller must wait if safe is occupied by 2 tellers

    //create tellers
    for (int i = 0; i < NO_TELLERS; i++) {
        pthread_create(&teller_threads[i], NULL, teller, (void*)(long)i);
    }
    //create customers
    for (int i = 0; i < NO_CUSTOMERS; i++) {
        pthread_create(&customer_threads[i], NULL, customer, (void*)(long)i);
    }
}