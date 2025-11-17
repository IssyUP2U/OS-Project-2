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
sem_t teller_ready; //teller is ready
sem_t door;
sem_t manager;
sem_t safe;

int idle_tellers[NO_TELLERS]; //a stack for idle teller IDs
int idle_top = 0;
int trans_type[NO_CUSTOMERS]; //store transaction types

int assigned_customer[NO_TELLERS];
pthread_mutex_t assign_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t idle_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rem_mutex = PTHREAD_MUTEX_INITIALIZER;
int customers_remaining = NO_CUSTOMERS; //track customers still waiting

sem_t customer_signal[NO_TELLERS]; //customer signals they are ready
sem_t customer_ask[NO_CUSTOMERS]; //teller asks for transaction
sem_t customer_tell[NO_CUSTOMERS]; //Deposit or Withdrawl
sem_t customer_end[NO_CUSTOMERS]; //transaction is over
sem_t customer_leave[NO_CUSTOMERS]; //the customer has left 

void logumentation(const char *type, int id, const char *bracket, int other_id, const char *msg) {
    if (bracket)
        printf("%s %d [%s %d]: %s\n", type, id, bracket, other_id, msg);
    else
        printf("%s %d: %s\n", type, id, msg);
    fflush(stdout);
}

void *customer(void *arg) {
    int cid = (int)(intptr_t)arg; //store customer id (arg) as an int
    int trans = (rand() % 2); //pick Deposit or Withdrawl
    trans_type[cid] = trans; //set transaction type

    int waitms = rand() % 101; //random wait time between 0 and 100
    usleep(waitms * 1000);

    sem_wait(&bank_opens); 
    sem_post(&bank_opens);

    sem_wait(&door); //enter door
    sem_wait(&tellers_open); //get idle teller

    pthread_mutex_lock(&idle_mutex);
    int tid = idle_tellers[--idle_top];
    pthread_mutex_unlock(&idle_mutex);

    pthread_mutex_lock(&assign_mutex);
    assigned_customer[tid] = cid;
    pthread_mutex_unlock(&assign_mutex);

    sem_post(&customer_signal[tid]); //customer is ready
    sem_wait(&customer_ask[cid]); //wait for the tellers question
    sem_post(&customer_tell[cid]); 
    sem_wait(&customer_end[cid]); //wait for end of transaction
    sem_post(&customer_leave[cid]); //signal customer is leaving
    sem_post(&door);

    pthread_mutex_lock(&rem_mutex);
    customers_remaining--;
    int rem = customers_remaining;
    pthread_mutex_unlock(&rem_mutex);

 
    if (rem == 0) { //if 0 remaining customers
        pthread_mutex_lock(&assign_mutex);
        for (int t = 0; t < NO_TELLERS; ++t) {
            assigned_customer[t] = -2;  
            sem_post(&customer_signal[t]); 
        }
        pthread_mutex_unlock(&assign_mutex);
    }


    return NULL;
}

void *teller(void *arg) {
    int tid = (int)(intptr_t)arg;  //store teller id (arg) as an int
    char buf[BUFF];

    sprintf(buf, "is ready to serve");
    logumentation("Teller", tid, NULL, -1, buf);
    sem_post(&teller_ready); //signal a teller is ready

    pthread_mutex_lock(&idle_mutex);
    idle_tellers[idle_top++] = tid; //add teller to stack and increment idle_top
    pthread_mutex_unlock(&idle_mutex);
    sem_post(&tellers_open);

    while (1) {
        sem_wait(&customer_signal[tid]); //wait for a customer

        pthread_mutex_lock(&assign_mutex);
        int cid = assigned_customer[tid];
        pthread_mutex_unlock(&assign_mutex);

        if (cid == -2) { //shutdown condition
            sprintf(buf, "is shutting down");
            logumentation("Teller", tid, NULL, -1, buf);
            break;
        }

        sprintf(buf, "asks customer %d for transaction type", cid);
        logumentation("Teller", tid, "Customer", cid, buf);
        sem_post(&customer_ask[cid]); //signal that customer is being asked something
        sem_wait(&customer_tell[cid]); //wait for customer response pluhhhh

        int trans = trans_type[cid];

        if (trans == 1) { // Withdrawal
            sprintf(buf, "is going to manager to ask for permission");
            logumentation("Teller", tid, "Customer", cid, buf);

            sprintf(buf, "is waiting to talk to the manager");
            logumentation("Teller", tid, "Manager", -1, buf);
            sem_wait(&manager);

            sprintf(buf, "is talking to the manager");
            logumentation("Teller", tid, "Manager", -1, buf);
            //teller and manager interaction
            int ms = 5 + rand() % 26;
            sprintf(buf, "is starting manager interaction (%d ms)", ms);
            logumentation("Teller", tid, "Manager", -1, buf);

            usleep(ms * 1000);

            sprintf(buf, "ending manager interaction");
            logumentation("Teller", tid, "Manager", -1, buf);

            sem_post(&manager);

            sprintf(buf, "is done with the manager");
            logumentation("Teller", tid, "Customer", cid, buf);
        }

        sprintf(buf, "is going to the safe");
        logumentation("Teller", tid, "Customer", cid, buf);

        sprintf(buf, "is waiting to enter the safe");
        logumentation("Teller", tid, "Safe", -1, buf);
        sem_wait(&safe); //wait for safe to have an open spot

        sprintf(buf, "in safe performing transaction");
        logumentation("Teller", tid, "Safe", -1, buf);

        int ms2 = 10 + rand() % 41; //10 to 50ms
        sprintf(buf, "is accessing the safe (%d ms)", ms2);
        logumentation("Teller", tid, "Customer", cid, buf);
        usleep(ms2 * 1000);

        sprintf(buf, "end transaction in safe");
        logumentation("Teller", tid, "Customer", cid, buf);

        sem_post(&safe); //leave safe

        sprintf(buf, "has left the safe");
        logumentation("Teller", tid, "Customer", cid, buf);

        sprintf(buf, "is telling the customer the transaction is complete");
        logumentation("Teller", tid, "Customer", cid, buf);
        sem_post(&customer_end[cid]);

        sprintf(buf, "is waiting for customer to leave");
        logumentation("Teller", tid, "Customer", cid, buf);
        sem_wait(&customer_leave[cid]);

        sprintf(buf, "the customer has left");
        logumentation("Teller", tid, "Customer", cid, buf);

        pthread_mutex_lock(&idle_mutex); //add teller to the idle queue
        idle_tellers[idle_top++] = tid;
        pthread_mutex_unlock(&idle_mutex);
        sem_post(&tellers_open);

        pthread_mutex_lock(&assign_mutex);
        if (assigned_customer[tid] != -2) 
            assigned_customer[tid] = -1;
            pthread_mutex_unlock(&assign_mutex);    
    }

    return NULL;
}

int main() {
    //initialize semaphoresssss
    sem_init(&bank_opens, 0, 0);
    sem_init(&tellers_open, 0, 0);
    sem_init(&teller_ready, 0, 0);
    sem_init(&door, 0, 2); //2 threads can enter the door
    sem_init(&manager, 0, 1); //1 thread can access the manager at a time
    sem_init(&safe, 0, 2); //teller must wait if safe is occupied by 2 tellers

    for (int i = 0; i < NO_TELLERS; i++)
        sem_init(&customer_signal[i], 0, 0);

    for (int i = 0; i < NO_CUSTOMERS; i++) {
        sem_init(&customer_ask[i], 0, 0);
        sem_init(&customer_tell[i], 0, 0);
        sem_init(&customer_end[i], 0, 0);
        sem_init(&customer_leave[i], 0, 0);
    }
    //initialization done

    for (int i = 0; i < NO_TELLERS; i++) assigned_customer[i] = -1; //all tellers set to unassigned
    //create tellers
    pthread_t teller_threads[NO_TELLERS];
    for (int i = 0; i < NO_TELLERS; i++)
        pthread_create(&teller_threads[i], NULL, teller, (void*)(long)i);

    for (int i = 0; i < NO_TELLERS; i++)
        sem_wait(&teller_ready); //wait for tellers to be ready before creating customers

    for (int i = 0; i < NO_CUSTOMERS; i++)
        sem_post(&bank_opens); //bank opens

    printf("The bank is now opened\n"); 

    pthread_t customer_threads[NO_CUSTOMERS];
    for (int i = 0; i < NO_CUSTOMERS; i++)
        pthread_create(&customer_threads[i], NULL, customer, (void*)(long)i);

    for (int i = 0; i < NO_CUSTOMERS; i++)
        pthread_join(customer_threads[i], NULL);

    for (int i = 0; i < NO_TELLERS; i++)
        pthread_join(teller_threads[i], NULL);

    printf("\nPROGRAM FINISHED\n");
    return 0;
}
