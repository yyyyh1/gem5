#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <sys/errno.h>
#include <sys/types.h>  
#include "m5op.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdalign.h>
#include<sys/time.h>

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500+
#endif /* __STDC_VERSION__ */

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))


/* This is the control panel of the benchmark */
#ifndef SCRIPT//This will remove all the control panel details when using the script

//        #define     Intel_PCM       1                   // Running on Intel PCM counters or not
        #define     P               2                   // Number of processors
        #define     N               64             // Square matrix dimension N*N
        #define     TILE            16                  // Tile size
        #define     NUMBER_OF_TRIALS    1               //Number of times to repeat the whole benchmark (this is to reduce results randomness)
        #define     USE_DYNAMIC_ARRAY   0               //This allocates the arrays dynamically using "new". This allows having huge matrices
        #define bool int
        #define true 1
        #define false 0
#endif ///end of ifndef SCRIPT
#if Intel_PCM
#include "cpucounters.h"        // Intell PCM monitoring tool
#endif
/*These are the variables that are not conflicting with the script*/
//#define     GEM5            0                   // Running on Gem5 or not
#define     DEBUG           0                   // Print debug results or not
#define     PRINT           0                   // Print results or not
#define     SIM_LIMIT       500024                   // Number of k2 loops to simulate
#define     OUTERTILE       256               // Tile size
#define     WARMUP_LIMIT    1                  // Number of k2 loops to use for warmup
#define     NUM_CP          0                   // Not applicable here
#define     NUM_BARRIERS    4                   // Number of barriers
#define     Benchmark_Mode  0                   //0: base, 1: Recompute, 2: Logging, 3: Naive Checkpointing

pthread_mutex_t SyncLock[NUM_BARRIERS];         /* mutex */
pthread_cond_t  SyncCV[NUM_BARRIERS];           /* condition variable */
int             SyncCount[NUM_BARRIERS];        /* number of processors at the barrier so far */


const int n     = N;                            // matrix dimension
const int tile  = TILE;		                    // tile size
int firstTime;                    
struct timespec t1[P]; 
struct timespec t2[P];
struct timespec d[P];


#if USE_DYNAMIC_ARRAY
    alignas(64) int **c, **a, **b;    // the two matrices a and b. c is the resultant matrix
#else
alignas(64) int c[N][N], a[N][N], b[N][N];    // the two matrices a and b. c is the resultant matrix
#endif


alignas(64) int lastJJ[P*16];         // the last computed iteration of k2 loop
alignas(64) int lastII[P*16];         	// the last computed iteration of i loop
alignas(64) int lastKK[P*16];         // the last computed iteration of k2 loop
alignas(64) int lastJJLog[P*16];        // log for the last computed iteration of i loop
alignas(64) int lastIILog[P*16];        // log for the last computed iteration of i loop
alignas(64) int lastKKLog[P*16];       // log for the last computed iteration of k2 loop
alignas(64) int insideTxII[P*16];               // Flag for entering logging region for updating II index
alignas(64) int insideTxKK[P*16];               // Flag for entering logging region for updating KK index
uint64_t start, end;                            // Used for printing the ticks on real system

// inline uint64_t rdtsc()
// {
//     unsigned long a, d;
//     asm volatile ("cpuid; rdtsc" : "=a" (a), "=d" (d) : : "ebx", "ecx");
//     return a | ((uint64_t)d << 32);
// }

void  diff(struct timespec * difference, struct timespec start, struct timespec end)
{
    if ((end.tv_nsec-start.tv_nsec)<0) {
        difference->tv_sec = end.tv_sec-start.tv_sec-1;
        difference->tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        difference->tv_sec = end.tv_sec-start.tv_sec;
        difference->tv_nsec = end.tv_nsec-start.tv_nsec;
    }
}

void Barrier(int idx) {
    int ret;

    pthread_mutex_lock(&SyncLock[idx]); /* Get the thread lock */
    SyncCount[idx]++;
    if(SyncCount[idx] == P) {
        ret = pthread_cond_broadcast(&SyncCV[idx]);
        assert(ret == 0);
    } else {
        ret = pthread_cond_wait(&SyncCV[idx], &SyncLock[idx]); 
        assert(ret == 0);
    }
    pthread_mutex_unlock(&SyncLock[idx]);
}

void Initialize() 
{
    int i, j;

    //    srand (time(NULL));    
    srand48(0);
    for(i=0; i<P; i++) {
        lastJJ[i*16] = (i*(n/P))-tile;
        lastII[i*16] = -tile;
        lastKK[i*16] = -tile;
        lastJJLog[i*16] = (i*(n/P))-tile;
        lastIILog[i*16] = -tile;
        lastKKLog[i*16] = -tile;
        insideTxII[i*16] = 0;
        insideTxKK[i*16] = 0;
    }

#if USE_DYNAMIC_ARRAY
    a = (int**)calloc(N,sizeof(int*));
    b = (int**)calloc(N,sizeof(int*));
    c = (int**)calloc(N,sizeof(int*));
    for(int i=0;i<N;i++)
    {
	a[i] = (int*)calloc(N,sizeof(int));
    b[i] = (int*)calloc(N,sizeof(int));
    c[i] = (int*)calloc(N,sizeof(int));
    }
#endif
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            //a[i][j] = drand48();
            //b[i][j] = drand48();
            a[i][j] = 2;
            b[i][j] = 3;
            c[i][j] = 0;
            //            a[i][j] = rand() % 20;
            //            b[ithreads][j] = rand() % 20;
            //            c[i][j] = 0;
        }
    }
}

void PrintC() 
{
    int i, j;

    printf("The C matrix:\n");
    for (i=0; i< n; i++) {
        for (j=0; j< n; j++) 
            //printf("%0.2f\t",c[i][j]); 
            printf("%d ",c[i][j]);
        printf("\n");
    }
    printf("\n");
}

void checkC() 
{
    int i, j;
    bool passed = true;
    for (i=0; i< n; i++) {
        for (j=0; j< n; j++) 
            //if(c[i][j] != ((SIM_LIMIT*16)*6.0)) printf("error in cell c[%d][%d]. It is %0.2f\n",i,j,c[i][j]); 
            if(c[i][j] != ((n)*6.0)) {printf("error in cell c[%d][%d]. It is %0.2f\n",i,j,c[i][j]);passed = false;} 
    }
    if(passed) printf("Passed all tests!\n");
}

void* multiply(void* tmp) {
    /* each thread has a private version of local variables */
    int     tid = (uintptr_t) tmp; 
    int     i, j, k, i2, j2, k2, r, l;  //iterators
    float     sum;                      //sum of multiplication
    int     firstLoop, lastLoop;

    firstLoop = tid*(n/P);
    lastLoop = firstLoop + (n/P);

    /*////////////////////////////////////////////////////////////////////////////////////////////\*/
    /*****************  Normal (i.e. not Recovery mode) tiled multiplication operation mode  **********************/
    /*////////////////////////////////////////////////////////////////////////////////////////////\*/
    for (k2=0; k2<n; k2+=tile) {
	/*if(k2 == tile*(WARMUP_LIMIT)) {//do warmup
            Barrier(0);
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1[tid]);
#if GEM5
            if(tid == 1) m5_dumpreset_stats(0,0);
#endif
            Barrier(1);
        }*/
        for (i2=0; i2<n; i2+=tile) {
            for (j2=firstLoop; j2<lastLoop; j2+=tile) {
                for (i=i2; i<min(i2+tile,n); i++) {
                    for(j=j2; j<min(j2+tile,n); j++) {
                        sum = c[i][j];             // initialize the value of the current element
                        for(k=k2; k<min(k2+tile,n); k++) {
                            sum += a[i][k]*b[k][j];// calculate the sum for this element
                        }
                        c[i][j] = sum;             // store the newly computed element value
                    } //end of for j
                } //end of for i
            } //end of for j2
        } //end of for i2
//                if(k2 == (tile*(SIM_LIMIT-1))) break;
    } //end of for k2

#if GEM5
    Barrier(2);
    if(tid == 1) m5_dumpreset_stats(0,0);
    Barrier(3);
#endif
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2[tid]);
}

int main()
{
#if DEBUG
        assert(N >= (P*TILE));//guarantee that there are enough cells to be distributed among threads
#endif

#if Intel_PCM
        PCM * m = PCM::getInstance();
        PCM::ErrorCode returnResult = m->program();
        if (returnResult != PCM::Success){
                std::cerr << "Intel's PCM couldn't start" << std::endl;
                std::cerr << "Error code: " << returnResult << std::endl; exit(1);
        }

        SystemCounterState before_sstate = getSystemCounterState();
#endif


for(int trial=0;trial<NUMBER_OF_TRIALS;trial++)
{

    pthread_t*     threads;
    pthread_attr_t attr;
    int            ret, dx;
    int i;
    struct timeval begin1, end1;
    int execTime;

    Initialize();

    /* Initialize array of thread structures */
    threads = (pthread_t *) malloc(sizeof(pthread_t) * P);
    assert(threads != NULL);

    /* Initialize thread attribute */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); // sys manages contention

    for(i=0; i<NUM_BARRIERS; i++) {
        /* Initialize mutexs */
        ret = pthread_mutex_init(&SyncLock[i], NULL);
        assert(ret == 0);

        /* Init condition variable */
        ret = pthread_cond_init(&SyncCV[i], NULL);
        assert(ret == 0);
        SyncCount[i] = 0;
    }


    /*
#if GEM5
m5_reset_stats(0,0);
    //    gettimeofday (&begin1, NULL);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
#else
start = rdtsc();
    //    gettimeofday (&begin1, NULL);
#endif
*/

    for(dx=0; dx < P; dx++) {
        ret = pthread_create(&threads[dx], &attr, multiply, (void*)(uintptr_t)dx);
        // printf("ret: %d", ret);
        assert(ret == 0);
        
    }

    /* Wait for each of the threads to terminate */
    for(dx=0; dx < P; dx++) {
        ret = pthread_join(threads[dx], NULL);
        assert(ret == 0);
    }

    for(i=0; i<P; i++) {
        diff(&d[i], t1[i], t2[i]);
        printf("thread %d) Execution Time: %ld sec, %ld nsec\n", i, d[i].tv_sec, d[i].tv_nsec);
    }
}
#if GEM5
    m5_exit(0);
#endif
#if Intel_PCM
        SystemCounterState after_sstate = getSystemCounterState();

        
	printf("%" PRIu64 "\tInstructions per clock\n",getIPC(before_sstate,after_sstate));
        printf("%" PRIu64 "\tInstructions Retired\n",getInstructionsRetired(before_sstate,after_sstate));
        printf("%" PRIu64 "\tCycles\n",getCycles(before_sstate,after_sstate));
        printf("%" PRIu64 "\tMC reads\n",getBytesReadFromMC(before_sstate,after_sstate)/64);
        printf("%" PRIu64 "\tMC writes\n",getBytesWrittenToMC(before_sstate,after_sstate)/64);
        printf("%" PRIu64 "\tBytes read from EDC\n",getBytesReadFromEDC(before_sstate,after_sstate));
        printf("%" PRIu64 "\tBytes written to EDC\n",getBytesWrittenToEDC(before_sstate,after_sstate));
        printf("%" PRIu64 "\tL3 misses\n",getL3CacheMisses(before_sstate,after_sstate));
        printf("%" PRIu64 "\tL3 hits\n",getL3CacheHits(before_sstate,after_sstate));
        m->cleanup();
	

#endif

	printf("c[0][0]=%d, c[%d][%d]=%d\n",c[0][0], N-1, N-1, c[N-1][N-1]);
    /*
#if GEM5
clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
diff(&d, t1, t2);
m5_dumpreset_stats(0,0);
printf("Execution Time: %ld sec, %ld nsec\n", d.tv_sec, d.tv_nsec);
    //    gettimeofday (&end1, NULL);
    //    execTime = 1e6 * (end1.tv_sec - begin1.tv_sec) + (end1.tv_usec - begin1.tv_usec);
    //    printf("time it took: %d\n", execTime);
    m5_exit(0);
#else
    //    gettimeofday (&end1, NULL);
    //    execTime = 1e6 * (end1.tv_sec - begin1.tv_sec) + (end1.tv_usec - begin1.tv_usec);
    //    printf("time it took: %d\n", execTime);
    end = rdtsc();
    printf("%ld ticks\n", end - start);
#endif
*/
#if DEBUG2
	checkC();
#endif

#if PRINT
    PrintC();
#endif

    return 0;
}
