/*
* Matthew Buchanan
* CS-314 Operating Systems
* Summer 2018
* Project 1: Binary space partition memory management simulation
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#define FALSE 0
#define TRUE 1
#define DEBUG TRUE

/* Structure to simulate memory block */
typedef struct block
{
	int base;
	int limit;
	int blockSize;
	int pID; //starts at 1
	int freeMemory;//boolean to represent whether a block is free
	double fragmentation;
	struct block* previousNeighbor;
	struct block* nextNeighbor;
} Block;

/* Function prototypes */
Block* splitBlock(int size, Block *block);
int allocateMem(int size, int *pid, Block *head);
int freeMem(int pid, Block *head);
void report(Block *head);


/* Beginning of main */
int main(int argc, char* argv[])
{
	Block *head;//head pointer for list of Blocks
	head = calloc(1, sizeof(Block));
	head->freeMemory = 1;

	if (2 > argc) {
		printf("Wrong number of command line arguments, program terminating. Press any key to continue...\n");
		getchar();
		exit(1);
	}
	int memSize = atoi(argv[1]);
	int j = ceil(log2(memSize));
	int k = floor(log2(memSize));
	if (j != k || memSize < 2) { // check if memsize is at least 2 and is a power of 2
		printf("Invalid memory size, program terminating. Press any key to continue...\n");
		getchar();
		exit(1);
	}
	head->blockSize = memSize;

	char command;
	int commandQuantity;
	int counter = 1; //process id's start at 1
	do {
		printf("Enter a command: ");
		if (scanf(" %c", &command) == -1)
			printf("No command given, please try again\n");
		else{
			if (command == 'L' || command == 'F' || command == 'R' || command == 'Q') {
				if (command == 'R')
					report(head);
				else {
					if (command != 'Q')
						scanf(" %d", &commandQuantity);
					if (command == 'L') {
						if (!allocateMem(commandQuantity, &counter, head))
							printf("Allocation unsucessful, no suitable block found\n\n");
						else printf("\n");
					}
					if (command == 'F') {
						if (commandQuantity < 1 || commandQuantity > counter)
							printf("Impossible process id\n\n");
						if (freeMem(commandQuantity, head))
							printf("Process terminated and memory freed\n\n");
						else 
							printf("Process not found\n\n");
					}
				}
			}
			else {
				printf("command is: %c\n", command);
				printf("Your command was not recognized, please try again\n\n");
			}
		}
	} while ('Q' != command);
	printf("Program written by Matt Buchanan.\n");
}
/* End of main */


/* Function to simulate memory allocation */
int allocateMem(int size, int *pid, Block *head)
{
	Block *tmp = head;
	int found = FALSE;
	if (head->freeMemory && !head->nextNeighbor) { //empty list
		if (size > head->blockSize || size < 2)
			return FALSE; // not enough system memory to service request
		found = TRUE;
		if (head->blockSize >= 2 * size) // initial block must be split
			tmp = splitBlock(size, tmp);
	}
	else { // traverse list and search for large enough block
		while (tmp->nextNeighbor && !found) {
			tmp = tmp->nextNeighbor;
			if (tmp->freeMemory && tmp->blockSize >= size) { // if block is free and large enough
				if (tmp->blockSize >= 2 * size) // if block needs split
					tmp = splitBlock(size, tmp);
				found = TRUE;
			}
		}
	}
	if (found){ //allocate attributes to block
		tmp->pID = *pid;
		*pid = *pid + 1;
		tmp->freeMemory = 0;//mark block as occupied with a process
		tmp->limit = tmp->base + size - 1; //set limit for block
		tmp->fragmentation =  (1.0 - (double)(tmp->limit - tmp->base + 1) / tmp->blockSize);
		//output results of successful allocation
		printf("Process id: %d\n", tmp->pID);
		printf("Block size allocated: %d\n", tmp->blockSize);
		printf("Internal Fragmentation: %f\n", tmp->fragmentation);
		printf("Base: %d\n", tmp->base);
		printf("Limit: %d\n\n", tmp->limit - tmp->base + 1);//here limit should be size of process, misheard instructions
	}
	return found;
}

/* Function to split blocks in half until right size */
Block* splitBlock(int size, Block *block)
{
	Block *tmp1, *tmp2;
	tmp1 = NULL;
	tmp2 = block;
	while (tmp2->blockSize >= 2 * size) { //split the block and move forward one block
		if (block->nextNeighbor) // hold nextNeighbor for reattachment after split(s)
			tmp1 = block->nextNeighbor;
		tmp2->blockSize = tmp2->blockSize / 2;
		tmp2->nextNeighbor = calloc(1, sizeof(Block));
		tmp2->nextNeighbor->blockSize = tmp2->blockSize;
		tmp2->nextNeighbor->previousNeighbor = tmp2;
		tmp2->nextNeighbor->base = tmp2->base + tmp2->blockSize;// set base register for new block
		tmp2->nextNeighbor->limit = tmp2->base + tmp2->blockSize;
		tmp2->nextNeighbor->freeMemory = 1;//mark newly created block as free
		if (tmp1) { // reattach nextNeighbor
			tmp2->nextNeighbor->nextNeighbor = tmp1;
			tmp1->previousNeighbor = tmp2->nextNeighbor;
		}
	}
	return tmp2;
}

/* Function to simulate freeing of memory */
int freeMem(int pid, Block *head)
{
	Block *tmp = head;
	int found = FALSE;
	if (head->freeMemory && !head->nextNeighbor)//empty list
		return FALSE;
	if (tmp->pID == pid) { //head is desired process
		found = TRUE;
		tmp->freeMemory = 1;
		tmp->limit = tmp->base;
		tmp->pID = 0;
	}
	else{
		while (tmp->nextNeighbor && !found) { //search list for desired process
			tmp = tmp->nextNeighbor;
			if (tmp->pID == pid) {//free attributes of desired process
				found = TRUE;
				tmp->freeMemory = 1;
				tmp->limit = tmp->base;
				tmp->pID = 0;
			}
		}
	}
	if (found){ //The while loop runs while the previous or next neighbor of current block is it's free binary partner
		while (tmp->previousNeighbor && tmp->previousNeighbor->freeMemory && tmp->previousNeighbor->blockSize == tmp->blockSize && (tmp->previousNeighbor->base % (tmp->previousNeighbor->blockSize + tmp->blockSize) == 0)
			|| (tmp->nextNeighbor && tmp->nextNeighbor->freeMemory && tmp->nextNeighbor->blockSize == tmp->blockSize && (tmp->base % (tmp->nextNeighbor->blockSize + tmp->blockSize)) == 0)) { 
			//If this block's previous neighbor is it's binary partner and is free, combine
			if (tmp->previousNeighbor && tmp->previousNeighbor->freeMemory && tmp->previousNeighbor->blockSize == tmp->blockSize && (tmp->previousNeighbor->base % (tmp->previousNeighbor->blockSize + tmp->blockSize) == 0)) {
				Block *deleteLocation = *&tmp;
				tmp = tmp->previousNeighbor;
				tmp->blockSize *= 2;
				if (deleteLocation->nextNeighbor) {
					tmp->nextNeighbor = deleteLocation->nextNeighbor;
					deleteLocation->nextNeighbor->previousNeighbor = tmp;
				}
				else
					tmp->nextNeighbor = NULL;
				free(deleteLocation);
			}
			//If this block's next neighbor is it's binary partner and is free, combine
			if ((tmp->nextNeighbor && tmp->nextNeighbor->freeMemory && tmp->nextNeighbor->blockSize == tmp->blockSize && (tmp->base % (tmp->nextNeighbor->blockSize + tmp->blockSize)) == 0)) {
				Block *deleteLocation = *&tmp->nextNeighbor;
				tmp->blockSize *= 2;
				if (deleteLocation->nextNeighbor) {
					tmp->nextNeighbor = deleteLocation->nextNeighbor;
					deleteLocation->nextNeighbor->previousNeighbor = tmp;
				}
				else
					tmp->nextNeighbor = NULL;
				free(deleteLocation);
			}
		}
	}
	return found;
}

/* Function to display current state of memory system */
void report(Block *head)
{
	Block *tmp = head;
	int tmpl;
	int ml[31] = { 0 };//hold a count of each empty block size possible in 32 bits

	printf("\nSystem Report:\n\n");
	if (tmp->freeMemory) {
		tmpl =  log2((double)tmp->blockSize) - 1;
		ml[tmpl] += 1;
	}
	else {
		printf("Process id: %d\n", tmp->pID);
		printf("Block size allocated: %d\n", tmp->blockSize);
		printf("Process size: %d\n", tmp->limit - tmp->base);
		printf("Internal Fragmentation: %f\n\n", tmp->fragmentation);
	}
	while (tmp->nextNeighbor) {
		tmp = tmp->nextNeighbor;
		if (tmp->freeMemory) {
			tmpl = log2((double)tmp->blockSize) - 1;
			ml[tmpl] += 1;
		}
		else {
			printf("Process id: %d\n", tmp->pID);
			printf("Block size allocated: %d\n", tmp->blockSize);
			printf("Process size: %d\n", tmp->limit - tmp->base);
			printf("Internal Fragmentation: %f\n\n", tmp->fragmentation);
		}
	}
	double temp;
	for (int i = 0; i < 31; i++) {
		if (ml[i] != 0) {
			temp = i + 1.0;
			printf("Block size:  %f\n", pow(2.0, temp));
			printf("Number of free blocks:  %d\n\n", ml[i]);
		}
	}
}
