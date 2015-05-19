#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <unistd.h>

#define SHAREDMEMORYKEY 7777
#define SHAREDMEMORYSIZE 1024

void ErrorMessage(char *pcMessage)
{
	perror(pcMessage);
}

void *ReadSharedMemory(void)
{
	int iShareMemoryID; /* Share memory ID */
	key_t tSharedMemoryKey = SHAREDMEMORYKEY; /* Get the shared memory key */
	char *pcSharedMemoryAddress; /* To get the created shared memory address */
	int iShareMemoryFlagRW = IPC_CREAT | 0666; /* Shared memory R/W permission */

	/* Create the shared memory id by tSharedMemoryKey */
	if ((iShareMemoryID = shmget(tSharedMemoryKey, SHAREDMEMORYSIZE, iShareMemoryFlagRW)) < 0)
	{
		ErrorMessage("Create shared memory ID");
		pthread_exit(NULL);
	}

	/* Get the shared memory segment to our data space */
	/* shmat(shmid, assigned address for shared memory segment) */
	if ((pcSharedMemoryAddress = shmat(iShareMemoryID, NULL, iShareMemoryFlagRW)) == NULL)
	{
		ErrorMessage("Get the shared memory to our data space");
		pthread_exit(NULL);
	}

	while (1)
	{
		while (*pcSharedMemoryAddress == '\0')
		{
			usleep(10);
		}

		fprintf(stdout, "************************\n");
		fprintf(stdout, "Receive:\n");
		fprintf(stdout, "%s", pcSharedMemoryAddress);
		fprintf(stdout, "************************\n");

		*pcSharedMemoryAddress = '\0';
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int iShareMemoryID; 						/* Share memory ID */
	key_t tSharedMemoryKey = SHAREDMEMORYKEY; 	/* Get the shared memory key */
	char *pcSharedMemoryAddress; 				/* To get the created shared memory address */
	int iShareMemoryFlagRW = IPC_CREAT | 0666;	/* Shared memory R/W permission */
	unsigned int uiMessageLen;					/* Message string lenth */

	pthread_t tReadSharedMemory; /* pthread of read shared memory */
		
	/* Create the shared memory id by tSharedMemoryKey */
	if ((iShareMemoryID = shmget(tSharedMemoryKey, SHAREDMEMORYSIZE, iShareMemoryFlagRW)) < 0)
	{
		ErrorMessage("Create shared memory ID");
		return -1;
	}

	/* Get the shared memory segment to our data space */
	/* shmat(shmid, assigned address for shared memory segment) */
	if ((pcSharedMemoryAddress = shmat(iShareMemoryID, NULL, iShareMemoryFlagRW)) == NULL)
	{
		ErrorMessage("Get the shared memory to our data space");
		return -1;
	}

	/* Create pthread of read shared memory */
	pthread_create(&tReadSharedMemory, NULL, (void *)ReadSharedMemory, NULL);

	fprintf(stdout, "Ready to write something\n");

	while (1)
	{
		if (fgets(pcSharedMemoryAddress, SHAREDMEMORYSIZE, stdin) == NULL)
		{
			ErrorMessage("Failed to get string from stdin");
		}

		/* the input string lenth */
		uiMessageLen = strlen(pcSharedMemoryAddress);

		/* Check input string lenth overflower? */
		if (uiMessageLen >= SHAREDMEMORYSIZE)
		{
			pcSharedMemoryAddress[SHAREDMEMORYSIZE] = '\0';
		}

		/* Wiat the other process read shared memory and clear shared memory */
		while (*pcSharedMemoryAddress != '\0')
		{
			usleep(10);
		}
	}

	pthread_join(tReadSharedMemory, NULL);

	if ((shmctl(iShareMemoryID,IPC_RMID,0)) < 0)
	{
		fprintf(stderr, "Delete shared memory failed\n");
	}

	return 0;
}
