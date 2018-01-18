cd lib/
make
cd ..
gcc -std=gnu99 -I./lib -Wall -pthread -c hw4_mm_pthread_test.c -o hw4_mm_pthread_test.o
gcc -std=gnu99 -I./lib -Wall -pthread -o hw4_mm_pthread_test hw4_mm_pthread_test.o ./lib/hw_malloc.o
