#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BLOCK_MAX_SIZE 100

#define BLOCK_MAX_SIZE 100

int main(int argc, char ** argv) {
    if(argc != 3){
        fprintf(stderr, "Use %s  <key> <file>\n",argv[0]);
        return 1;
    }
    key_t key = atoi(argv[1]);
    if(key < 0){
        fprintf(stderr, "Error: key not created\n");
        return 2;
    }
    int sharedMemoryID = shmget(key,BLOCK_MAX_SIZE, 0666 );
    if(sharedMemoryID < 0){
        fprintf(stderr, "Error: shared memory not gotten\n");
        return 3;
    }
    void * memorySegment =  shmat(sharedMemoryID, NULL, 0);
    if(memorySegment  == (void *) -1){
        fprintf(stderr, "Error: shared memory segment not attached\n");
        return 4;
    }

    int outputFile = open(argv[2],O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(outputFile < 0){
        fprintf(stderr, "Error: Destination file not opened\n");
        return 5;
    }
    memcpy(memorySegment,"_IM_READY_" ,11);
    while(strcmp(memorySegment ,"_IM_READY_") == 0){
        sleep(1);
    }
    while(strcmp(memorySegment,"_END_") != 0){

        write(outputFile,memorySegment,strlen(memorySegment));

        memcpy(memorySegment,"_WAIT_",7 );
        while(strcmp(memorySegment ,"_WAIT_") ==0){
            sleep(1);
        }
    };



    if(close(outputFile)<0){
        fprintf(stderr, "Error: Destination file not closed\n");
        return 6;
    }
    memcpy(memorySegment,"_IM_OUT_",9 );
    if(shmdt(memorySegment) == -1){
        fprintf(stderr, "Error: shared memory segment not detached\n");
        return 7;
    }

    return 0;
}
