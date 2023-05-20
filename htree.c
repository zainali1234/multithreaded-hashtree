#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>     // for EINTR
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

// Print out the usage of the program and exit.
void usage(char*);
uint32_t jenkins_one_at_a_time_hash(const uint8_t* , uint64_t );

int numThreads = 0;
uint64_t sizeForEachThread = 0;
char *buffer;

// block size
#define BSIZE 4096

void *child( void *ptr )
{
    uint32_t n = (uint32_t) ptr;

    pthread_t thread1, thread2;
    bool child1 = false, child2 = false;
    if (2*n+1 < numThreads) {
        pthread_create(&thread1, NULL, child, (void *) (2*n+1));
        child1 = true;
        if (2*n+2 < numThreads) {
             pthread_create(&thread2, NULL, child, (void *) (2*n+2));
             child2 = true;
        }
     }

     char *ptr1 = NULL, *ptr2 = NULL;
     if (child1)
        pthread_join(thread1, &ptr1);
        //char results[100];
        //sprintf(results, "%u%u", hash, child1hash);

     if (child2)
        pthread_join(thread2, &ptr2);

      char *temp = (char *) malloc(100); // Allocate memory for temp
      if (temp == NULL)
      {
          perror("Memory allocation failure.");
          exit(EXIT_FAILURE);
      }

     //uint32_t total = 1 + (uint32_t) ptr1 + (uint32_t) ptr2;

     sprintf(temp, "%u%u%u", jenkins_one_at_a_time_hash(&buffer[n * sizeForEachThread], sizeForEachThread), (uint32_t) ptr1, (uint32_t) ptr2);
     uint32_t total = jenkins_one_at_a_time_hash(temp, strlen(temp));

     printf("Thread %d: %u\n", n, total);
     return (void *) total;
}


int main(int argc, char** argv)
{
  int32_t fd;
  uint32_t nblocks;

  // input checking
  if (argc != 3)
    usage(argv[0]);

  // open input file
  fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    perror("open failed");
    exit(EXIT_FAILURE);
  }
  // use fstat to get file size
  struct stat fileInfo;

  if (fstat(fd, &fileInfo)) {
    perror("File not found");
    exit(EXIT_FAILURE);
  }

  // calculate nblocks
  uint64_t fileSize = fileInfo.st_size;

  nblocks = fileSize / BSIZE;

  if (fileSize % BSIZE) {
     nblocks++;
  }

  printf("no. of blocks = %u \n", nblocks);

  buffer = (char *) malloc (fileSize);
  if (buffer == NULL) {
                perror("Memory allocation failure.");
                exit(EXIT_FAILURE);
  }
  uint64_t nread = 0;
  ssize_t n = 0;
  while ((nread < fileSize) && ((n = read(fd, &buffer[nread], fileSize-nread)) > 0)) {
        nread += n;
        //n = read(fd, &buffer[nread], fileSize-nread);
  }

  if (n < 0) {
            perror("File read error.");
            exit(EXIT_FAILURE);
  }

  clock_t start = clock(); 

  pthread_t thread1;

  numThreads = atoi(argv[2]);

  sizeForEachThread = fileSize / numThreads;

  uint32_t i = 0;
  pthread_create(&thread1, NULL, child, (void *) i);

  void *ptr;
  pthread_join(thread1, &ptr);
  //printf("%u\n", (uint32_t) ptr);

  clock_t end = clock();
  //uint32_t hash = 0;
  //printf("hash value = %u \n", hash);
  printf("time taken = %f \n", (end - start) * 1.0 / CLOCKS_PER_SEC);
  close(fd);
  return EXIT_SUCCESS;
}

uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, uint64_t length)
{
  uint64_t i = 0;
  uint32_t hash = 0;

  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

void usage(char* s)
{
  fprintf(stderr, "Usage: %s filename num_threads \n", s);
  exit(EXIT_FAILURE);
}
