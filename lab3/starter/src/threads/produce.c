// Use this to see if a number has an integer square root
#define EPS 1.E-7


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

double g_time[2];

struct CircleQueue
{
	int front, rear;
	int capacity;
	int *array;
}

struct CircleQueue* init(int size)
{
	struct CircleQueue* queue = malloc(sizeof(struct CircleQueue));

	if(!queue)
		return NULL;

	q->capacity = size;
	q->front = -1;
	q->rear = -1;
	q->array = malloc(q->capacity*sizeof(int));

	if(!q->array)
		return NULL;
}

void enqueue(struct CircleQueue* queue, int x)//produce
{
	if((queue->rear+1)%queue->capacity == q->front)
	{
		// queue full, block, wait
	}
	else
	{
		q->rear = (q->rear+1)%q->capacity;
		q->array[q->rear] = x;
		if(q->front == -1)
			q->front = q->rear;
	}
}

int dequeue(struct CircleQueue* queue)//consume
{
	int data;
	if(q->front == -1)
		return 0;
	else
	{
		data = q->array[q->front];
		if(q->front == q->rear)
			q->front = q->rear = -1;
		else
			q->front = (q->front+1)%q->capacity;
	}
	return data;
}

int main(int argc, char *argv[])
{
	int num;
	int maxmsg;
	int num_p;
	int num_c;
	int i;
	struct timeval tv;

	if (argc != 5) {
		printf("Usage: %s <N> <B> <P> <C>\n", argv[0]);
		exit(1);
	}

	num = atoi(argv[1]);	/* number of items to produce */
	maxmsg = atoi(argv[2]); /* buffer size                */
	num_p = atoi(argv[3]);  /* number of producers        */
	num_c = atoi(argv[4]);  /* number of consumers        */


	gettimeofday(&tv, NULL);
	g_time[0] = (tv.tv_sec) + tv.tv_usec/1000000.;


    	gettimeofday(&tv, NULL);
    	g_time[1] = (tv.tv_sec) + tv.tv_usec/1000000.;

    printf("System execution time: %.6lf seconds\n", \
            g_time[1] - g_time[0]);
	exit(0);
}
