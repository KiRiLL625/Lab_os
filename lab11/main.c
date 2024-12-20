#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_READERS 10
#define ARRAY_SIZE 100

char shared_array[ARRAY_SIZE];
pthread_rwlock_t rwlock;
int counter = 0;
int stop = 0;

void* writer_thread(void* arg) {
    while (1) {
        pthread_rwlock_wrlock(&rwlock);
        if (counter >= 10) {
            stop = 1;
            pthread_rwlock_unlock(&rwlock);
            break;
        }
        snprintf(shared_array + counter * 2, ARRAY_SIZE - counter * 2, "%d ", counter);
        printf("Array updated at index: %d\n", counter);
        counter++;
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
    return NULL;
}

void* reader_thread(void* arg) {
    int reader_id = *(int*)arg;
    pthread_t tid = pthread_self();
    while (1) {
        pthread_rwlock_rdlock(&rwlock);
        if (stop) {
            pthread_rwlock_unlock(&rwlock);
            break;
        }
        printf("Reader (id[%d]) reads array: [%s]| tid: [%lx]\n", reader_id, shared_array, tid);
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t writer;
    pthread_t readers[NUM_READERS];
    int reader_ids[NUM_READERS];

    pthread_rwlock_init(&rwlock, NULL);

    // Создание потока записи
    pthread_create(&writer, NULL, writer_thread, NULL);

    // Создание потоков чтения
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i;
        pthread_create(&readers[i], NULL, reader_thread, &reader_ids[i]);
    }

    // Ожидание завершения потока записи
    pthread_join(writer, NULL);

    // Уведомление всех потоков чтения о завершении
    pthread_rwlock_wrlock(&rwlock);
    stop = 1;
    pthread_rwlock_unlock(&rwlock);

    // Ожидание завершения потоков чтения
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    pthread_rwlock_destroy(&rwlock);
    return 0;
}