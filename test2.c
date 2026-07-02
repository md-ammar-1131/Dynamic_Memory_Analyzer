#include <stdlib.h>
int main() {
// 1. Normal malloc & free (Clean)
void *ptr1 = malloc(1024);
free(ptr1);
// 2. Calloc (Intentional Leak - 1024 bytes)
void *ptr2 = calloc(4, 256);
// 3. Realloc (Clean)
void *ptr3 = malloc(128);
ptr3 = realloc(ptr3, 512);
free(ptr3);
// 4. Malloc (Intentional Leak - 2048 bytes)
void *ptr4 = malloc(2048);
// 5. Double Free Anomaly
void *ptr5 = malloc(64);
free(ptr5); // First free (valid)
free(ptr5); // Second free (DOUBLE FREE)
return 0;
}