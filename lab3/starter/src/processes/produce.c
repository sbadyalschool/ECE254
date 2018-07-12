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
#include <stdbool.h>

double g_time[2];

const char* Q_NAME = "/sqrt_q";

int N; // number of numbers
int B; // buffer size
int P; // number of producers
int C; // number of consumers

void consumer(int id);
void producer(int id);

int main(int argc, char *argv[])
{
    // record start time
    struct timeval tv;
    double t1;
    double t2;
    gettimeofday(&tv, NULL);
    t1 = tv.tv_sec + tv.tv_usec/1000000.0;

    // check if valid args
    if ( argc !=5 ) {
        printf("Arguments: %s <N> <B> <P> <C> \n", argv[0]);
        exit(1);
    }

    N = atoi(argv[1]);
    B = atoi(argv[2]);
    P = atoi(argv[3]);
    C = atoi(argv[4]);

    // initializing queue attr
    mqd_t qdes;
    struct mq_attr attr;
    attr.mq_maxmsg = B;
    attr.mq_msgsize = sizeof(int);
    attr.mq_flags = 0;
    attr.mq_curmsgs = 0;

    // make queue, set the mode permissions
    //taken from documentation 
    qdes = mq_open(Q_NAME, (O_CREAT | O_RDWR), (S_IRWXG | S_IRWXO | S_IRWXU), &attr);

    //error state queue failed to open 
    if (qdes == -1) {
        perror("message queue failed at the beggining");
        return 1;
    }

    // save pids
    pid_t* prod_pids = malloc(P * sizeof(pid_t));
    pid_t* con_pids = malloc(C * sizeof(pid_t));

    int i;
    // Create consumers
    for (i = 0; i < C; i++) {
        con_pids[i] = fork();
        if (con_pids[i] < 0) {
            printf("xonsumer %d fork failed\n", i);
        } else if (con_pids[i] == 0) {
            consumer(i);
            break;
        }
    }
    // create producers
    for (i = 0; i < P; i++) {
        prod_pids[i] = fork();
        if (prod_pids[i] < 0) {
            printf("producer %d fork failed\n", i);
        } else if (prod_pids[i] == 0) {
            producer(i);
            break;
        }
    }
    // let producers finish
    int* ret_val;
    for(i = 0; i < P; i++) {
        //wait on all producer processes 
        waitpid(prod_pids[i], ret_val, 0);
    }
    // kill all consumers
    int kill = -1;
    for (i = 0; i < C; i++) {
        if (mq_send(qdes,(char *) &kill, sizeof(int), 0) == -1) {
            perror("message queue send failed ");
            exit(1);
        }
    }
    // Wait on consumers
    for(i = 0; i < C; i++) {
        //wait on all consumer processes 
        waitpid(con_pids[i], ret_val, 0);
    }
    //  cleanup
    if (mq_close(qdes) == -1) {
        perror("message queue close failed ");
        exit(2);
    } else if (mq_unlink(Q_NAME) != 0){
        perror("message queue unlink failed");
        exit(3);

    }
    //memory cleanup
    free(prod_pids);
    free(con_pids);

    gettimeofday(&tv, NULL);

    t2 = tv.tv_sec + tv.tv_usec/1000000.0;
    printf("System execution time: %.6lf seconds\n", t2-t1);

    return 0;
}

void consumer(int cid) {
    // reading
    mqd_t q = mq_open(Q_NAME, O_RDWR);
    if (q == -1) {
        perror("message queue failed in consumer");
        exit(1);
    }
    int pt;
    int root;
    while (true){
        // error check
        if (mq_receive(q, (char *) &pt, sizeof(int), NULL) == -1) {
            perror("message queue failed ");
            printf("consumer: %d failed.\n", cid);
            exit(1);
        } else {
            // happy case
            if (pt < 0) {
                break;
            }

            // check squareroot
            root = sqrt(pt);
            if (root*root == pt) {
                printf("%d, %d, %d\n", cid, pt, root);
            }
        }
    }
    mq_close(q);
    exit(0);
}

void producer(int id) {
    int i;

    // write
    mqd_t q = mq_open(Q_NAME, O_WRONLY);
    if (q == -1) {
        perror("producer message queue failed ");
        exit(1);
    }

    for (i = 0; i < N; i++) {
        if (i%P == id) {
            if (mq_send(q,(char *) &i, sizeof(int), 0) == -1) {
                perror("producer message queue failed ");
                exit(1);
            }
        }
    }
    mq_close(q);
    exit(0);
}
