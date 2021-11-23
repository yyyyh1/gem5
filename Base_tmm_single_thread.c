#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include<sys/time.h>

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#ifndef SCRIPT
    #define     P               2                   // Number of processors
    #define     N               64             // Square matrix dimension N*N
    #define     TILE            16                  // Tile size
    #define     NUMBER_OF_TRIALS    1               //Number of times to repeat the whole benchmark (this is to reduce results randomness)
    #define     USE_DYNAMIC_ARRAY   1               //This allocates the arrays dynamically using "new". This allows having huge matrices
    #define bool int
    #define true 1
    #define false 0
#endif ///end of ifndef SCRIPT

const int n     = N;                            // matrix dimension
const int tile  = TILE;		                    // tile size
int firstTime;                    
struct timespec t1; 
struct timespec t2;
struct timespec d;

#if USE_DYNAMIC_ARRAY
alignas(64) int **c, **a, **b;    // the two matrices a and b. c is the resultant matrix
#else
alignas(64) int c[n][n], a[n][n], b[n][n];    // the two matrices a and b. c is the resultant matrix
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
            //            b[i][j] = rand() % 20;
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

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
}


int main()
{

    for(int trial=0;trial<NUMBER_OF_TRIALS;trial++){
        
        int dx = 0;
        int i;
        int execTime;
        Initialize();

        multiply((void*)(uintptr_t)dx);
        diff(&d, t1, t2);
        printf(" Execution Time: %ld sec, %ld nsec\n", d.tv_sec, d.tv_nsec);
        
    }
    printf("c[0][0]=%d, c[%d][%d]=%d\n",c[0][0], N-1, N-1, c[N-1][N-1]);
    //checkC();
}   

