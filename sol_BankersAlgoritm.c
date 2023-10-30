//Project work is to make a Multithreaded Program that implements the banker's algorithm.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

int nResources,
    nProcesses;
int *resources;
int **allocated;
int **maxRequired;
int **need;
int *safeSeq;
int nProcessRan = 0;

pthread_mutex_t lockResources;
pthread_cond_t condition;

// geting safe sequence if there is one else returning false
bool getSafeSeq();
// function of the process is given below
void* processCode(void* );

int main(int argc, char** argv) {
	srand(time(NULL));

        printf("\n Number of process you want to give: ");
        scanf("%d", &nProcesses);

        printf("\n Number of Resources you want to give: ");
        scanf("%d", &nResources);

        resources = (int *)malloc(nResources * sizeof(*resources));
        printf("\n Available resources (R1 R2 R3 ...) ");
        for(int i=0; i<nResources; i++)
                scanf("%d", &resources[i]);

        allocated = (int **)malloc(nProcesses * sizeof(*allocated));
        for(int i=0; i<nProcesses; i++)
                allocated[i] = (int *)malloc(nResources * sizeof(**allocated));

        maxRequired = (int **)malloc(nProcesses * sizeof(*maxRequired));
        for(int i=0; i<nProcesses; i++)
                maxRequired[i] = (int *)malloc(nResources * sizeof(**maxRequired));

        // allocated
        printf("\n");
        for(int i=0; i<nProcesses; i++) {
                printf("\n Allocated Resources to the process %d (R1 R2 ...) ", i+1);
                for(int j=0; j<nResources; j++)
                        scanf("%d", &allocated[i][j]);
        }
        printf("\n");

	// We are calculating the maximum required resources
        for(int i=0; i<nProcesses; i++) {
                printf("\nMaximum no. of resource required by process %d (R1 R2 ...) ", i+1);
                for(int j=0; j<nResources; j++)
                        scanf("%d", &maxRequired[i][j]);
        }
        printf("\n");

	// calculating the need matrix
        need = (int **)malloc(nProcesses * sizeof(*need));
        for(int i=0; i<nProcesses; i++)
                need[i] = (int *)malloc(nResources * sizeof(**need));

        for(int i=0; i<nProcesses; i++)
                for(int j=0; j<nResources; j++)
                        need[i][j] = maxRequired[i][j] - allocated[i][j];

	// Here we are getting the safe sequence
	safeSeq = (int *)malloc(nProcesses * sizeof(*safeSeq));
        for(int i=0; i<nProcesses; i++) safeSeq[i] = -1;

        if(!getSafeSeq()) {
                printf("\n Sorry BRO! The processes leads the system to a unsafe state.\n\n");
                exit(-1);
        }

        printf("\n\nSafe Sequence Found : ");
        for(int i=0; i<nProcesses; i++) {
                printf("%-3d", safeSeq[i]+1);
        }

        printf("\nExecuted Processes are: \n\n");
        sleep(1);
	
	// running threads
	pthread_t processes[nProcesses];
        pthread_attr_t attr;
        pthread_attr_init(&attr);

	int processNumber[nProcesses];
	for(int i=0; i<nProcesses; i++) processNumber[i] = i;

        for(int i=0; i<nProcesses; i++)
                pthread_create(&processes[i], &attr, processCode, (void *)(&processNumber[i]));

        for(int i=0; i<nProcesses; i++)
                pthread_join(processes[i], NULL);

        printf("\n All Processes are Finished, Thankyou..\n");	
	
	// freeing the resources
        free(resources);
        for(int i=0; i<nProcesses; i++) {
                free(allocated[i]);
                free(maxRequired[i]);
		free(need[i]);
        }
        free(allocated);
        free(maxRequired);
	free(need);
        free(safeSeq);
}


bool getSafeSeq() {
	// getting safe sequence
        int tempRes[nResources];
        for(int i=0; i<nResources; i++) tempRes[i] = resources[i];

        bool finished[nProcesses];
        for(int i=0; i<nProcesses; i++) finished[i] = false;
        int nfinished=0;
        while(nfinished < nProcesses) {
                bool safe = false;

                for(int i=0; i<nProcesses; i++) {
                        if(!finished[i]) {
                                bool possible = true;

                                for(int j=0; j<nResources; j++)
                                        if(need[i][j] > tempRes[j]) {
                                                possible = false;
                                                break;
                                        }

                                if(possible) {
                                        for(int j=0; j<nResources; j++)
                                                tempRes[j] += allocated[i][j];
                                        safeSeq[nfinished] = i;
                                        finished[i] = true;
                                        ++nfinished;
                                        safe = true;
                                }
                        }
                }

                if(!safe) {
                        for(int k=0; k<nProcesses; k++) safeSeq[k] = -1;
                        return false; // no safe sequence found
                }
        }
        return true; // safe sequence found
}

// main process code
void* processCode(void *arg) {
        int p = *((int *) arg);

	// lock resources
        pthread_mutex_lock(&lockResources);

        // condition check
        while(p != safeSeq[nProcessRan])
                pthread_cond_wait(&condition, &lockResources);

	// process
        printf("\n--> Process %d", p+1);
        printf("\n\tAllocated : ");
        for(int i=0; i<nResources; i++)
                printf("%3d", allocated[p][i]);

        printf("\n\tNeeded    : ");
        for(int i=0; i<nResources; i++)
                printf("%3d", need[p][i]);

        printf("\n\tAvailable : ");
        for(int i=0; i<nResources; i++)
                printf("%3d", resources[i]);

        printf("\n"); sleep(1);

        printf("\tResources Allocated!");
        printf("\n"); sleep(1);
        printf("\tProcess Code Running, Plzz wait.");
        printf("\n"); sleep(rand()%3 + 2); // process code
        printf("\tProcess Code Completed.ThankYou Mam.");
        printf("\n"); sleep(1);
        printf("\tProcess Releasing Resources.");
        printf("\n"); sleep(1);
        printf("\tResource Released, Have a great day");

	for(int i=0; i<nResources; i++)
                resources[i] += allocated[p][i];

        printf("\n\tNew Available : ");
        for(int i=0; i<nResources; i++)
                printf("%3d", resources[i]);
        printf("\n\n");

        sleep(1);

	// condition broadcast
        nProcessRan++;
        pthread_cond_broadcast(&condition);
        pthread_mutex_unlock(&lockResources);
	pthread_exit(NULL);
}