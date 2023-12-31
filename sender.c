#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define BLOCK_MAX_SIZE 100

int main(int argc, char ** argv) {
    if(argc != 2){
        fprintf(stderr, "Use %s <file>\n",argv[0]);
        return 1;
    }

    key_t key = ftok(argv[1],'a');
    if(key < 0){
        fprintf(stderr, "Error: key not created\n");
        return 2;
    }
    printf("Key: %d\n",key);
    int sharedMemoryID = shmget(key,BLOCK_MAX_SIZE, 0666 | IPC_CREAT | IPC_EXCL );
    if(sharedMemoryID < 0){
        fprintf(stderr, "Error: shared memory not gotten\n");
        return 3;
    }
    void * memorySegment =  shmat(sharedMemoryID, NULL, 0);
    if(memorySegment  == (void *) -1){
        fprintf(stderr, "Error: shared memory segment not attached\n");
        return 4;
    }

    int inputFile = open(argv[1],O_RDONLY);
    if(inputFile < 0){
        fprintf(stderr, "Error: Source file not opened\n");
        return 5;
    }

    while(strcmp(memorySegment ,"_IM_READY_") != 0){
        sleep(1);
    }
    char buff[BLOCK_MAX_SIZE];
    int readInf = read(inputFile,buff,BLOCK_MAX_SIZE);
    while(readInf > 0){
        if(readInf < BLOCK_MAX_SIZE) buff[readInf] = '\0';
        memcpy(memorySegment,buff,readInf);
        readInf = read(inputFile,buff,BLOCK_MAX_SIZE);
        while(strcmp(memorySegment ,"_WAIT_") != 0){
            sleep(1);
        }
    }
    while(strcmp(memorySegment ,"_WAIT_") != 0){
        sleep(1);
    }
    memcpy(memorySegment,"_END_",6 );

    if(close(inputFile)<0){
        fprintf(stderr, "Error: Source file not closed\n");
        return 6;
    }

    while(strcmp(memorySegment ,"_IM_OUT_") != 0){
        sleep(1);
    }

    if(shmdt(memorySegment) == -1){
        fprintf(stderr, "Error: shared memory segment not detached\n");
        return 7;
    }

    if(shmctl (sharedMemoryID, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Error: shared memory not removed\n");
        return 8;
    }

    return 0;
}
