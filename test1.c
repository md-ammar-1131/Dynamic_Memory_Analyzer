#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// This function will be run by our extra threads
void* thread_work(void* arg) {
    // Each thread allocates and frees 500 bytes
    // Pin will intercept these malloc/free calls via your tool
    void* ptr = malloc(500);
    if (ptr == NULL) return NULL; 
    
    printf("Thread %lu: Allocated 500 bytes\n", (unsigned long)pthread_self());
    
    free(ptr);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    printf("Main Thread: Starting execution\n");

    // Thread 0 (main) allocates 1000 bytes
    void* main_ptr = malloc(1000);
    if (main_ptr == NULL) return 1;

    // Create Thread 1 and Thread 2
    if (pthread_create(&thread1, NULL, thread_work, NULL) != 0) {
        perror("Failed to create thread 1");
        return 1;
    }
    if (pthread_create(&thread2, NULL, thread_work, NULL) != 0) {
        perror("Failed to create thread 2");
        return 1;
    }

    // Wait for Thread 1 and 2 to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Thread 0 frees its memory
    free(main_ptr);
    
    printf("Main Thread: Finished execution\n");

    return 0;
}