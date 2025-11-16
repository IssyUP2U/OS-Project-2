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
#define BUFF 128

sem_t bank_opens; //bank opens when all tellers are "ready to serve"
sem_t tellers_open; //count tellers that are idle
sem_t door;
sem_t manager;
sem_t safe;

int idle_tellers[NO_TELLERS]; //stack for idle teller IDs
int idle_top = 0; //top of stack

// teller & customer communication
sem_t customer_signal[NO_TELLERS]; //customer signals they are ready
sem_t customer_ask[NO_CUSTOMERS]; //teller asks for transaction
sem_t customer_tell[NO_CUSTOMERS];  //Deposit or Withdrawl
sem_t customer_end[NO_CUSTOMERS]; //transaction is over
sem_t customer_leave[NO_CUSTOMERS]; //the customer has left

void logumentation(const char *type, int id, const char *bracket, int other_id, const char *msg) {
    if (bracket)
        printf("%s %d [%s %d]: %s\n", type, id, bracket, other_id, msg);
    else
        printf("%s %d: %s\n", type, id, msg);
    fflush(stdout);
}

//customer routine
void *customer(void *arg) {}
//teller routine
void *teller(void *arg) {
    int tid = (int)(intptr_t)arg; //store teller id (arg) as an int
    char buf[BUFF];
    sprintf(buf, "is ready to serve");
    logumentation("Teller", tid, NULL, -1, buf);
    
}

int main( ) {
    srand(time(NULL));
    int r = rand() % 10; //random number between 0 and 9

    //initialize semaphoresssss
    sem_init(&door, 0, 2); //2 threads can enter the door
    sem_init(&manager, 0, 1); //1 thread can access the manager at a time
    sem_init(&safe, 0, 2); //teller must wait if safe is occupied by 2 tellers

    //create tellers
    pthread_t teller_threads[NO_TELLERS];
    for (int i = 0; i < NO_TELLERS; i++) {
        pthread_create(&teller_threads[i], NULL, teller, (void*)(long)i);
    }
    //create customers
    pthread_t customer_threads[NO_CUSTOMERS];
    for (int i = 0; i < NO_CUSTOMERS; i++) {
        pthread_create(&customer_threads[i], NULL, customer, (void*)(long)i);
    }
    //waiting for customers to be created
    for (int i = 0; i < NO_CUSTOMERS; i++) {
        pthread_join(customer_threads[i], NULL);
    }
    //waiting for tellers to be created
    for (int i = 0; i < NO_TELLERS; i++) {
        pthread_join(teller_threads[i], NULL);
    }

}
