#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define NUM_CHAIRS 3
#define NUM_STUDENTS 5

int waiting_students = 0;
int next_seat = 0;
int next_to_help = 0;

pthread_mutex_t mutex;
sem_t students_sem;
sem_t ta_sem;

void* student(void* num);
void* ta(void*);

int main() {
    srand(time(NULL));

    pthread_t ta_thread;
    pthread_t student_threads[NUM_STUDENTS];

    pthread_mutex_init(&mutex, NULL);
    sem_init(&students_sem, 0, 0);
    sem_init(&ta_sem, 0, 0);

    pthread_create(&ta_thread, NULL, ta, NULL);

    for(int i = 0; i < NUM_STUDENTS; i++) {
        int* student_id = (int*)malloc(sizeof(int));
        *student_id = i + 1;
        pthread_create(&student_threads[i], NULL, student, (void*)student_id);
    }

    pthread_join(ta_thread, NULL);

    for(int i = 0; i < NUM_STUDENTS; i++) {
        pthread_join(student_threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&students_sem);
    sem_destroy(&ta_sem);

    return 0;
}

void* student(void* num) {
    int id = *(int*)num;
    free(num);

    while(1) {
        sleep(rand() % 5 + 1);

        pthread_mutex_lock(&mutex);

        if (waiting_students < NUM_CHAIRS) {
            waiting_students++;
            printf("Student %d takes a seat. Waiting students = %d\n", id, waiting_students);
            
            sem_post(&students_sem);
            pthread_mutex_unlock(&mutex);

            sem_wait(&ta_sem);

            printf("Student %d is getting help from the TA\n", id);
            sleep(rand() % 3 + 1);
            printf("Student %d has been helped by the TA\n", id);
        } else {
            printf("Student %d found no available seat and will try again later\n", id);
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}

void* ta(void* arg) {
    while(1) {
        sem_wait(&students_sem);

        pthread_mutex_lock(&mutex);

        if (waiting_students > 0) {
            waiting_students--;
            sem_post(&ta_sem);
            pthread_mutex_unlock(&mutex);

            printf("TA is helping a student. Waiting students = %d\n", waiting_students);
            sleep(rand() % 3 + 1);
            printf("TA has finished helping a student\n");
        } else {
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}