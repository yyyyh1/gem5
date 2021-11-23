#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define   N         1*128       // Square matrix dimension N*N
#define   TILE      16          // Tile size
#define   OUTERTILE     64         // Tile size
#define   P         1           // Number of processors


const int innerTile  = TILE;		          // tile size
const int outerTile = OUTERTILE;
int c[N][N], a[N][N], b[N][N];  // the two matrices a and b. c is the resultant matrix
int lastJJ[P*16];     // the last computed iteration of k2 loop
int lastII[P*16];     	// the last computed iteration of i loop
int lastKK[P*16];     // the last computed iteration of k2 loop
int lastJJLog[P*16];    // log for the last computed iteration of i loop
int lastIILog[P*16];    // log for the last computed iteration of i loop
int lastKKLog[P*16];     // log for the last computed iteration of k2 loop
int insideTxII[P*16];         // Flag for entering logging region for updating II index
int insideTxKK[P*16];         // Flag for entering logging region for updating KK index

void Initialize() 
{
  int i, j;
  //  srand (time(NULL));  
  srand(0);
  for(i=0; i<P; i++) {
    lastJJ[i*16] = (i*(N/P))-innerTile;
    lastII[i*16] = -innerTile;
    lastKK[i*16] = -innerTile;
    lastJJLog[i*16] = (i*(N/P))-innerTile;
    lastIILog[i*16] = -innerTile;
    lastKKLog[i*16] = -innerTile;
    insideTxII[i*16] = 0;
    insideTxKK[i*16] = 0;
  }
/*
  a = new int*[N];
  b = new int*[N];
  c = new int*[N];
  for(int i=0;i<N;i++)
  {
    a[i] = new int[N];
    b[i] = new int[N];
    c[i] = new int[N];
  }
*/
  for (i=0; i<N; i++) {
    for (j=0; j<N; j++) {
      //a[i][j] = drand48();
      //b[i][j] = drand48();
      a[i][j] = 2;
      b[i][j] = 3;
      c[i][j] = 0.0;
    }
  }
}



int main (int argc, char *argv[])
{

Initialize();
printf("Initialization Done.\n");
int   i, j, k, i2, j2, k2, r, l, k3, i3, j3;  //iterators
float   sum;            //sum of multiplication
int   firstLoop, lastLoop;

firstLoop = 0;
lastLoop = firstLoop + (N/P);

  for(k3 =0; k3<N; k3+= outerTile){
    for(i3=0;i3<N; i3+=outerTile){
      for(j3=firstLoop;j3<lastLoop; j3+= outerTile){
      printf("k3 %d, i3 %d, j3 %d.\n",k3,i3,j3);
        ////Below are simply the normal TMM loops we had before (now for inner tiling)
        for (k2=k3; k2<(k3+outerTile); k2+=innerTile) {
          /*          if(k2 == innerTile*(WARMUP_LIMIT)) {//do warmup
                      printf("before barrier0---tid=%d, Here, k3=%d,i3=%d,j3=%d,k2=%d,i2=%d,j2=%d,k=%d,i=%d,j=%d,  \n",tid, k3, i3, j3, k2, i2, j2, k, i, j);
                      Barrier(0);
                      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1[tid]);
#if GEM5
if(tid == 1) m5_dumpreset_stats(0,0);
#endif
printf("before barrier1---tid=%d, Here, k3=%d,i3=%d,j3=%d,k2=%d,i2=%d,j2=%d,k=%d,i=%d,j=%d,  \n",tid, k3, i3, j3, k2, i2, j2, k, i, j);
Barrier(1);
}*/
        for (i2=i3; i2<(i3+outerTile); i2+=innerTile) {
          for (j2=j3; j2<(j3+outerTile); j2+=innerTile) {
            for (i=i2; i<(i2+innerTile); i++) {
              for(j=j2; j<(j2+innerTile); j++) {
                sum = c[i][j];       // initialize the value of the current element
                for(k=k2; k<(k2+innerTile); k++) {
                  sum += a[i][k]*b[k][j];// calculate the sum for this element
                }
                c[i][j] = sum;       // store the newly computed element value
                //	      printf("---tid=%d, Here, k3=%d,i3=%d,j3=%d,k2=%d,i2=%d,j2=%d,k=%d,i=%d,j=%d,  \n",tid, k3, i3, j3, k2, i2, j2, k, i, j);
              } //end of for j
            } //end of for i
          } //end of for j2
        } //end of for ii
//if(k2 == (innerTile*(SIM_LIMIT-1))) break;
} //end of for k2
}//end of j3
}//end of i3
//if(k3 == (outerTile*(SIM_LIMIT-1))) break;
}//end of k3


//printf("before barrier2---tid=%d, Here, k3=%d,i3=%d,j3=%d,k2=%d,ii=%d,j2=%d,k=%d,i=%d,j=%d,  \n",tid, k3, i3, j3, k2, ii, j2, k, i, j);
printf("Hello World!\n");
}
