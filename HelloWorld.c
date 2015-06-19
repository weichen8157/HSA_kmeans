#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//此行是snack.sh hw.cl 所產生的  必須加入！！！！！！ hw.h
#include "hw.h"
/*our main*/
#include <stdint.h>
#include <time.h>
#include "cl_version.c"
#include "c_version.c"

// param define
#define DCNT     30 /* 資料個數   */
#define DIM      10 /* 資料維度   */
#define K        3   /* 叢聚個數   */
#define MAX_ITER 1000  /* 最大迭代   */
#define MIN_PT   0   /* 最小變動點 */
#define LOW      1000  /* 資料下限   */
#define UP       300 /* 資料上限   */

// ------------------------------------
// variable
float data[DCNT][DIM]; /* 原始資料   */
float cent[K][DIM]; /* 重心       */
float centc[K][DIM]; /* 重心       */
float dis_k[K][DIM];   /* 叢聚距離   */
int table[DCNT];        /* 資料所屬叢聚*/
int cent_c[K];          /* 該叢聚資料數*/
float dis_k_c[K][DIM];   /* 叢聚距離   */
int table_c[DCNT];        /* 資料所屬叢聚*/
int cent_c_c[K];          /* 該叢聚資料數*/

// ------------------------------------
// function declare

float cal_dis(float *x, float *y, int dim);
void   get_data();               // 取得資料
void   kmeans_init();            // 演算法初始化
float update_table_cl(int* ch_pt); // 更新table
float update_table_c(int* ch_pt_c); // 更新table
void   update_cent_cl();            // 更新重心位置
void   update_cent_c();
void   print_cent_c();             // 顯示重心位置
void   print_cent_cl();             // 顯示重心位置


//void initial_kernel();
static inline void tic(struct timespec *t1);
static inline void toc(char *str, struct timespec *t1, struct timespec *t2);


int main(int argc, char* argv[]) {

       	/*our code*/
        int     ch_pt,ch_pt_c;         /* 紀錄變動之點 */
    int     iter=0,iter_c=0;        /* 迭代計數器   */
    float sse1,sse1_c;           /* 上一迭代之sse */
    float sse2,sse2_c;           /* 此次迭代之sse */
    struct timespec timer_1, timer_2;

    srand((unsigned)time(NULL));     


    //get_data();                      /* step 0 - 取得資料            */
    FILE *fp = fopen("/home/paslab/CLOC/examples/snack/test_snack/file.txt","r");
    
    int q,qq;
    for( q =0; q< DCNT ;++q)
    {
        for ( qq=0;qq<DIM; ++qq)
        {
                fscanf(fp,"%f,",&data[q][qq]);
        }
    }
    fclose(fp); 
    printf("11111111111; %f \n",data[0][1]);    
       
    kmeans_init();                   /* step 1 - 初始化,隨機取得重心 */
    
    /*OpenCL*/
    //initial_kernel();
    
    tic(&timer_1);

    SNK_INIT_LPARM(lparm,DCNT);
    toc("SNACK initial Time: ", &timer_1, &timer_2);
    
    tic(&timer_1);

    int i, j, k, min_k;
    float dis,  t_sse=0.0;

    float data_ker[DCNT*DIM];
    float cent_ker[K*DIM];
    int cent_c_ker[DCNT];

    //float min_dis;
    int chpt[DCNT];
    float min_dis[DCNT];
    unsigned int k_ker=K,dim_ker=DIM,dcnt_ker=DCNT;

    ch_pt=0;
    memset(cent_c, 0, sizeof(cent_c)); // ?叢?è?
    memset(dis_k, 0, sizeof(dis_k));   // ?叢?è?
 size_t work_size;

 for (i=0; i<DCNT;++i) {
          for(k=0; k<DIM;++k) {
                data_ker[i*DIM+k]=data[i][k];
          }
 }
  for (i=0; i<K;++i) {
          for(k=0; k<DIM;++k) {
                  cent_ker[i*DIM+k]=cent[i][k];
        }
}

cal_diskernel(data_ker,cent_ker,table,K,DIM,DCNT,chpt,cent_c_ker,min_dis,lparm);

 for(i=0;i<DCNT;i++) {
         ch_pt+=chpt[i];
         ++cent_c[cent_c_ker[i]];
          t_sse+=min_dis[i];
          for(j=0;j<DIM;j++) {
                   dis_k[table[i]][j]+=data[i][j];
                    }
        }
sse2=t_sse;


//    sse2 = update_table_cl(&ch_pt);     /* step 2 - 更新一次對應表      */
    do{
        sse1 = sse2, ++iter;
        update_cent_cl();             /* step 3 - 更新重心            */
       
       t_sse=0.0;
       ch_pt=0;
       memset(cent_c, 0, sizeof(cent_c)); // ?<84>??<9a>è?
       memset(dis_k, 0, sizeof(dis_k));   // ?<84>??<9a>è?
       size_t work_size;
       for (i=0; i<DCNT;++i) {
                for(k=0; k<DIM;++k) {
                           data_ker[i*DIM+k]=data[i][k];
                                    }
                             }
       for (i=0; i<K;++i) {
                for(k=0; k<DIM;++k) {
                           cent_ker[i*DIM+k]=cent[i][k];
                                     }
                          }
//SNK_INIT_LPARM(lparm,DCNT);
cal_diskernel(data_ker,cent_ker,table,K,DIM,DCNT,chpt,cent_c_ker,min_dis,lparm);
for(i=0;i<DCNT;i++) {
        ch_pt+=chpt[i];
       ++cent_c[cent_c_ker[i]];
       t_sse+=min_dis[i];
       for(j=0;j<DIM;j++) {
           dis_k[table[i]][j]+=data[i][j];
                          }
  }
  sse2=t_sse;

        
//     sse2=update_table_cl(&ch_pt); /* step 4 - 更新對應表          */
    }while(iter<MAX_ITER && sse1!=sse2 && ch_pt>MIN_PT); // 收斂條件
    toc("HSA Execution Time: ", &timer_1, &timer_2);

    print_cent_cl(); // 顯示最後重心位置
    printf("sse   = %.2lf\n", sse2);
    printf("ch_pt = %d\n", ch_pt);
    printf("iter = %d\n", iter);
/*    
    //our snack code
    float * data_ker=(float*)malloc(DCNT*DIM);
    float * cent_ker=(float*)malloc(K*DIM);
    int * table=(int*)malloc(DCNT);
    unsigned int * k_ker=(unsigned int*)malloc(K);
    unsigned int * dim_ker=(unsigned int*)malloc(DIM);
    unsigned int * dcnt_ker=(unsigned int*)malloc(DCNT);
    int * chpt=(int*)malloc(DCNT);
    int * cent_c_ker=(int*)malloc(DCNT);
    float * min_dis=(float*)malloc(DCNT);

    cal_diskernel(data_ker,cent_ker,table,K,DIM,DCNT,chpt,cent_c_ker,min_dis,lparm);
    cal_diskernel(data_ker,cent_ker,table,K,DIM,DCNT,chpt,cent_c_ker,min_dis,lparm);
*/      

//hw.h 裡面有type   extern _CPPSTRING_ void cal_diskernel
/*		(float* data_ker,
		float* cent_ker,
		int* table,
		const unsigned int K,
		const unsigned int DIM,
		const unsigned int DCNT,
		int* chpt,
		int* cent_c_ker,
		float* min_dis, 
		const snk_lparm_t * lparm);

*/

    /*C*/
    tic(&timer_1);
    sse2_c=update_table_c(&ch_pt_c); 
    do{
        sse1_c = sse2_c, ++iter_c;
        update_cent_c();             /* step 3 - 更新重心            */
        sse2_c=update_table_c(&ch_pt_c); /* step 4 - 更新對應表          */
    }while(iter_c<MAX_ITER && sse1_c!=sse2_c && ch_pt_c>MIN_PT); // 收斂條件
    toc("CPU Execution Time: ", &timer_1, &timer_2);
    print_cent_c(); // 顯示最後重心位置
    printf("sse_c   = %.2lf\n", sse2_c);
    printf("ch_pt_c = %d\n", ch_pt_c);
    printf("iter_c = %d\n", iter_c);

#ifdef __WINDOWS__
    system("PAUSE");
#endif
    return 0;

}//end main

//our function
// ------------------------------------
// 取得資料，此處以隨機給
/*
void get_data()
{
    int i, j;
    
    for(i=0; i<DCNT; ++i)
        for(j=0; j<DIM; ++j)
            data[i][j] = \
                         LOW + (float)rand()*(UP-LOW) / RAND_MAX;
    
    //input file.txt
   FILE *fp = fopen("file.txt","r");
   //int row, column;
    
   if(fp==NULL) {
        printf("Fail To Open File file.txt!!");
        return;
   }
                                       
   fscanf(fp, "%d,%d\n", &DCNT, &DIM);
   for (i = 0; i < DCNT; ++i) 
   { 
        for (j = 0; j < DIM; ++j) 
        {
                fscanf(fp, "%f,", &data[i][j]);
        }
   }       



}*/



// ------------------------------------
// 演算化初始化
void kmeans_init()
{
    int i, j, k, rnd;
    int pick[K];
    // 隨機找K 個不同資料點
    for(k=0; k<K; ++k){
        rnd = rand() % DCNT; // 隨機取一筆
        for(i=0; i<k && pick[i]!=rnd; ++i);
        if(i==k) pick[k]=rnd; // 沒重覆
        else --k; // 有重覆, 再找一次
    }
    // 將K 個資料點內容複制到重心cent
    for(k=0; k<K; ++k)
        for(j=0; j<DIM; ++j)
        {cent[k][j] = data[pick[k]][j];
            centc[k][j] = data[pick[k]][j];}
}
static inline void tic(struct timespec *t1)
{
    clock_gettime(CLOCK_REALTIME, t1);
}

static inline void toc(char *str, struct timespec *t1, struct timespec *t2)
{
    long period = 0;

    clock_gettime(CLOCK_REALTIME, t2);
    period = t2->tv_nsec - t1->tv_nsec;
    if (period < 0) 
            period +=(t2->tv_sec - t1->tv_sec-1)*1000000000+ 1000000000;
    else
    period += (t2->tv_sec - t1->tv_sec) * 1000000000;
    printf("\033[1;32m"); //Color Code - Green 
    printf("%s: %0.3lf ms\n\n", str, (double)period / 1000000.0);
    printf("\033[0;00m"); //Terminate Color Code
}


