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
#include <pthread.h>
#include <semaphore.h>

typedef struct
{
	int front, rear;
	int capacity;
	int *array;
} circle_queue_t;

typedef struct
{
	int id;
	int max;
} argument_t;

circle_queue_t* circle_queue_init(int size);
void enqueue(circle_queue_t* queue, int x);
int dequeue(circle_queue_t* queue);

/*Global variables */
double g_time[2];
int produce_count;
int consume_count = 0;
pthread_mutex_t produce_mutex;
pthread_mutex_t consume_mutex;
pthread_mutex_t buffer_mutex;
sem_t items;
sem_t spaces;
circle_queue_t* buffer;

void* produce(void* params)
{
	argument_t* argument = (argument_t*) params;
	int p_id = argument->id;
	//printf("Producer thread %d created\n", p_id);
	while(1)
	{
		//printf("Producing %d\n", p_id);
		pthread_mutex_lock(&produce_mutex);
		produce_count--;
		if(produce_count < 0)
		{
			pthread_mutex_unlock(&produce_mutex);
			return NULL;
		}
		pthread_mutex_unlock(&produce_mutex);
		sem_wait(&spaces);
		pthread_mutex_lock(&buffer_mutex);
		enqueue(buffer, p_id);
		pthread_mutex_unlock(&buffer_mutex);
		sem_post(&items);
		p_id += argument->max;
	}
}

void* consume(void* params)
{
	argument_t* argument = (argument_t*) params;
	int c_id = argument->id;
	//printf("Consumer thread %d created\n", c_id);
	while(1)
	{
		pthread_mutex_lock(&consume_mutex);
		consume_count++;
		if(consume_count > argument->max)
		{
			pthread_mutex_unlock(&consume_mutex);
			return NULL;
		}
		pthread_mutex_unlock(&consume_mutex);
		sem_wait(&items);
		pthread_mutex_lock(&buffer_mutex);
		int result = dequeue(buffer);
		pthread_mutex_unlock(&buffer_mutex);
		sem_post(&spaces);
		//printf("Consuming %d\n", result);
		int squareroot = (int) sqrt(result);
		if((squareroot * squareroot) == result)
		{
			printf("%d, %d, %d\n", c_id, result, squareroot);
		}
	}
	
}

int main(int argc, char *argv[])
{
	int num;
	int maxmsg;
	int num_p;
	int num_c;
	int i;
	struct timeval tv;
	pthread_t* producer_thread_array;
	pthread_t* consumer_thread_array;

	if (argc != 5) {
		printf("Usage: %s <N> <B> <P> <C>\n", argv[0]);
		exit(1);
	}

	num = atoi(argv[1]);	/* number of items to produce */
	maxmsg = atoi(argv[2]); /* buffer size                */
	num_p = atoi(argv[3]);  /* number of producers        */
	num_c = atoi(argv[4]);  /* number of consumers        */
	//printf("%d, %d, %d, %d\n", num, maxmsg, num_p, num_c);

	produce_count = num;
	/* Initialize mutexes, semaphores, and buffer */
	pthread_mutex_init(&produce_mutex, NULL);
	pthread_mutex_init(&consume_mutex, NULL);
	pthread_mutex_init(&buffer_mutex, NULL);
	sem_init(&items, 0, 0);
	sem_init(&spaces, 0, maxmsg);
	buffer = circle_queue_init(maxmsg);

	/* Allocate mem to pthread arrays */
	producer_thread_array = malloc(sizeof(*producer_thread_array) * num_p);
	consumer_thread_array = malloc(sizeof(*consumer_thread_array) * num_c);

	/* Begin Execution */	
	gettimeofday(&tv, NULL);
	g_time[0] = (tv.tv_sec) + tv.tv_usec/1000000.;

	for(i = 0; i < num_p; i++)
	{
		argument_t* arg = malloc(sizeof(argument_t));
		arg->id = i;
		arg->max = num_p;
		pthread_create(producer_thread_array + i, NULL, produce, (void*)arg);
	}

	for(i = 0; i < num_c; i++)
	{
		argument_t* arg = malloc(sizeof(argument_t));
		arg->id = i;
		arg->max = num;
		pthread_create(consumer_thread_array + i, NULL, consume, (void*)arg);
	}

	for(i = 0; i < num_p; i++)
	{
		pthread_join(*producer_thread_array + i, NULL);
	}

	for(i = 0; i < num_c; i++)
	{
		pthread_join(*consumer_thread_array + i, NULL);
	}
	/* End Execution */
    	gettimeofday(&tv, NULL);
    	g_time[1] = (tv.tv_sec) + tv.tv_usec/1000000.;

    printf("System execution time: %.6lf seconds\n", \
            g_time[1] - g_time[0]);

	pthread_mutex_destroy(&produce_mutex);
	pthread_mutex_destroy(&consume_mutex);
	pthread_mutex_destroy(&buffer_mutex);
	sem_destroy(&items);
	sem_destroy(&spaces);
	exit(0);
}

circle_queue_t* circle_queue_init(int size)
{
	circle_queue_t* q = malloc(sizeof(circle_queue_t));

	if(!q)
		return NULL;

	q->capacity = size;
	q->front = -1;
	q->rear = -1;
	q->array = malloc(q->capacity*sizeof(int));

	if(!q->array)
		return NULL;
	return q;
}

void enqueue(circle_queue_t* q, int x)//produce, protected
{
	/* if((q->rear+1)%q->capacity == q->front)
	{
		// queue full, block, wait
	}
	else
	{ */
	q->rear = (q->rear+1)%q->capacity;
	q->array[q->rear] = x;
	if(q->front == -1)
		q->front = q->rear;
	// }
}

int dequeue(circle_queue_t* q)//consume, protected
{
	int data;
	/* if(q->front == -1)
	{
		return 0;
	}
	else
	{ */
	data = q->array[q->front];
	if(q->front == q->rear)
		q->front = q->rear = -1;
	else
		q->front = (q->front+1)%q->capacity;
	// }
	return data;
}

