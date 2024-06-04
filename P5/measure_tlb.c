#include "measure_tlb.h"
#include <sys/time.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

//Magni Mar Magnason - magni21
//Thorir Hrafn Hardarson - thorirhh21

#define PAGESIZE 4096

uint64_t getTimeStamp() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	uint64_t time=0;
	time += tv.tv_sec*1000000;
	time += tv.tv_usec;
	return time;
}

uint64_t measureFunction( void(*function)(void *), void *arg ) {
	(void)function;
	(void)arg;
	// Implement this function
	uint64_t startTime;
	uint64_t endTime;

	startTime = getTimeStamp();
	function(arg);
	endTime = getTimeStamp();
	
	uint64_t difference = endTime - startTime;
	
	return difference;
}

void measureStatistics( int statcount, Statistics *stat, void(*function)(void *), void*arg) {
	(void)statcount;
	(void)function;
	(void)arg;

	uint64_t curr;
	uint64_t total;
	uint64_t first = measureFunction(function, arg);

	stat->min = first;
	total = first;
	stat->max = first;

	int i=0;

	for (i=0; i<statcount-1; i++) 
	{
		
		curr = measureFunction(function, arg);
	
		if (stat->min > curr) 
		{
			stat->min = curr;
		}
	
		if (curr > stat->max) 
		{
			stat->max = curr;
		}
		
		total += curr;
	
	}
	
	if (statcount > 0) { 
		stat->avg = total / statcount;
	}
	else {
		stat->avg = 0;
	}
}

void accessMemory(int n, int pages) {
	(void)n;
	(void)pages;
	// Implement this function
	int i;
	int pageNum = 0;
	
	char *pagePtr = malloc(pages*PAGESIZE);
	if (pagePtr==NULL)
	{
		printf("insufficient memory for %d pages \n", pages);
		exit(1);
	}
	else
	{
		for (i=0; i<n; i++)
		{
			pagePtr[pageNum*PAGESIZE] = pageNum;
			if (pageNum == (pages-1)) 
			{
				pageNum = 0;
			}
			else 
			{
				pageNum++;
			}
		}
	}
	free(pagePtr);
}

void accessMemoryWrapper(void *p) {
	(void)p;
	// Implement this function
	MeasurementParameters* temp = p;
	accessMemory(temp->iterations, temp->pages);
}