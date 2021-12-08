/*
    m5threads, a pthread library for the M5 simulator
    Copyright (C) 2009, Stanford University
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/



#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <sys/time.h>
#include <inttypes.h>
// without volatile, simulator test works even if __thread support is broken
__thread volatile int local = 7;

volatile long long int jjjs = 0;

static const int count = 1024;

#define     P               2                   // Number of processors
#define     N               64             // Square matrix dimension N*N
#define     TILE            16                  // Tile size


const int n     = N; 
const int tile  = TILE;
// alignas(64) int **c, **a, **b;
alignas(64) int c[n][n], a[n][n], b[n][n]; 

alignas(64) int lastJJ[P*16];         // the last computed iteration of k2 loop
alignas(64) int lastII[P*16];         	// the last computed iteration of i loop
alignas(64) int lastKK[P*16];         // the last computed iteration of k2 loop
alignas(64) int lastJJLog[P*16];        // log for the last computed iteration of i loop
alignas(64) int lastIILog[P*16];        // log for the last computed iteration of i loop
alignas(64) int lastKKLog[P*16];       // log for the last computed iteration of k2 loop
alignas(64) int insideTxII[P*16];               // Flag for entering logging region for updating II index
alignas(64) int insideTxKK[P*16];               // Flag for entering logging region for updating KK index
uint64_t start, end;                            // Used for printing the ticks on real system


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

// #if USE_DYNAMIC_ARRAY
//     a = (int**)calloc(N,sizeof(int*));
//     b = (int**)calloc(N,sizeof(int*));
//     c = (int**)calloc(N,sizeof(int*));
//     for(int i=0;i<N;i++)
//     {
//         a[i] = (int*)calloc(N,sizeof(int));
//         b[i] = (int*)calloc(N,sizeof(int));
//         c[i] = (int*)calloc(N,sizeof(int));
//     }
// #endif
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


void* run (void* arg)
{
    long long int id = (long long int)arg;
    int i;
    printf("&local[%lld]=%p\n", id, &local);
    local += id;
    for (i = 0; i < count; i++) {
        local++;
    }

    //Some calculations to delay last read
    long long int jjj = 0;
    for (i = 0; i < 10000; i++) {
      jjj = 2*jjj +4 -i/5 + local;
    }
    jjjs = jjj;
 
    //assert(local == count +id);
    return (void*)local;
}

int main (int argc, char** argv)
{
    // if (argc != 2) { 
    //     printf("usage: %s <thread_count>\n", argv[0]);
    //     exit(1);
    // }
    // int P = atoi(argv[1]);
    assert(N >= (P*TILE));
    printf("Starting %d threads...\n", P);

    //struct timeval startTime;
    //int startResult = gettimeofday(&startTime, NULL);
    //assert(startResult == 0);
    int i;
    pthread_attr_t attr;

    Initialize();

    pthread_t* threads = (pthread_t*)calloc(P, sizeof(pthread_t));
    assert(threads != NULL);

    /* Initialize thread attribute */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);//sys manages contention

    for (i = 1 ; i < P; i++) {
        int createResult = pthread_create(&threads[i], 
                                          NULL,
                                          run,
                                          (void*)i);
        assert(createResult == 0);
    }

    long long int local = (long long int)run((void*)0);
    printf("local[0] = %lld\n", local);

    for (i = 1 ; i < P; i++) {
        int joinResult = pthread_join(threads[i], 
                                      (void**)&local);
        assert(joinResult == 0);
        printf("local[%d] = %lld\n", i, local);
    }
    
    /*struct timeval endTime;
    int endResult = gettimeofday(&endTime, NULL);
    assert(endResult == 0);
    
    long startMillis = (((lfine     N               64             // Square matrix dimension N*N
#definong)startTime.tv_sec)*1000) + (((long)startTime.tv_usec)/1000);
    long endMillis   = (((long)endTime.tv_sec)*1000)   + (((long)endTime.tv_usec)/1000);
    */
    /*printf("End Time (s)    = %d\n", (int)endTime.tv_sec);
    printf("Start Time (s)  = %d\n", (int)startTime.tv_sec);
    printf("Time (s)        = %d\n", (int)(endTime.tv_sec-startTime.tv_sec));
    printf("\n");
    printf("End Time (us)   = %d\n", (int)endTime.tv_usec);
    printf("Start Time (us) = %d\n", (int)startTime.tv_usec);
    printf("Time (us)       = %d\n", (int)(endTime.tv_usec-startTime.tv_usec));
    printf("\n");
    printf("End Time (ms)   = %d\n", (int)endMillis);
    printf("Start Time (ms) = %d\n", (int)startMillis);
    printf("Time (ms)       = %d\n", (int)(endMillis-startMillis));
    printf("\n");*/

    /*double difference=(double)(endTime.tv_sec-startTime.tv_sec)+(double)(endTime.tv_usec-startTime.tv_usec)*1e-6;
    printf("Time (s) = %f\n", difference);
    printf("\n");*/
    return 0;
}
