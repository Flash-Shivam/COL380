#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>
#include <pthread.h>

#define e 3

void matrix_mult(int n, double *A[n], double *B[n],double *result[n]);
void matrix_sub(int n, double *A[n], double *B[n],double *result[n]);
void print_matrix(int n, double *A[n]);
double euclidean_norm(int n, double *A[n]);


int step = 0;

int inc;

int q; //dummy variable to check boundary condition .

double a[e][e],pi[e],l[e][e],u[e][e],temp_col[e],temp_row[e];

pthread_mutex_t mutex;

void *myThreadFun(void* y)
{
    int k_prime, k,i,j;

    int start1 = inc * step++;
    int start;
    int start2 = start1+inc;
    int *r = (int*)y;
    if(q == *r)
    {
       start2 = e;
    }

    printf("%d %d\n",start1,start2);
    for(start=start1;start<start2;start++){
      k = start;

    double max=0;
    for(i=start;i<e;i++)
    {
      if(max<abs(a[i][k]))
			{
				max=abs(a[i][k]);
				k_prime=i;
			}
    }

    double tmp=pi[k_prime];

    pthread_mutex_lock(&mutex);

    pi[k_prime]=pi[k];
    pi[k]=tmp;

    pthread_mutex_unlock(&mutex);

    for (i = 0; i < e; i++)
		{

			tmp=a[k][i];
			pthread_mutex_lock(&mutex);
			a[k][i]=a[k_prime][i];
			a[k_prime][i]=tmp;
      pthread_mutex_unlock(&mutex);

		}

    for (i = 0; i < k; i++)
		{
			  tmp=l[k][i];
		    pthread_mutex_lock(&mutex);
				l[k][i]=l[k_prime][i];
				l[k_prime][i]=tmp;
			  pthread_mutex_unlock(&mutex);
		}

    pthread_mutex_lock(&mutex);
    u[k][k]=a[k][k];
    for(int t=k+1;t<e;t++)
    {
      temp_row[t]=a[k][t];
      temp_col[t]=a[t][k];
    }
    pthread_mutex_unlock(&mutex);

    for (i = k+1; i < e; i++)
    {
      l[i][k]=temp_col[i]/u[k][k];
      u[k][i]=temp_row[i];
    }

    for (i = k+1; i < e; i++)
		{
			for (j = k+1; j < e; j++)
			{
				pthread_mutex_lock(&mutex);
				a[i][j]-=l[i][k]*u[k][j];
        pthread_mutex_unlock(&mutex);
			}
		}

  }

}

int main()
{
	int n;
	printf("Enter the size of the matrix: ");
	// scanf("%d",&n);
  n = e;
	int no_threads;
	printf("Enter the no. of threads to be used: ");
	scanf("%d",&no_threads);
	printf("\n");

  inc = e/no_threads;
  q = no_threads;
	// omp_set_num_threads(no_threads);
  //
	// clock_t t;
	// t=clock();

	double x;
  // 	struct drand48_data randBuffer;
  //
  // 	srand48_r(time(NULL), &randBuffer);
  	int i,j;

	// double *a[n];
  //   	for (i=0; i<n; i++)
  //       	a[i] = (double *)malloc(n * sizeof(double));

	double *c[n];
	for (i=0; i<n; i++)
        	c[i] = (double *)malloc(n * sizeof(double));

	// #pragma omp parallel for private(j,x)
	for(i=0;i<n;i++)
	{
		// printf("%d\n", omp_get_thread_num());
		for(j=0;j<n;j++)
		{

				// drand48_r(&randBuffer, &x);
        scanf("%lf",&x);
				a[i][j] = x;
				c[i][j] = a[i][j];

		}
	}

	// print_matrix(n, a);
	// printf("\n");

	// double *pi;
	// pi = (double *)malloc(n*sizeof(double));

	// #pragma omp parallel for
	for(i=0;i<n;i++)
	{
		pi[i]=i;
	}

	// double *u[n];
	// for (i=0; i<n; i++)
  //       	u[i] = (double *)malloc(n * sizeof(double));

	// #pragma omp parallel for private(j)
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			u[i][j]=0;
		}
	}

	// double *l[n];
	// for (i=0; i<n; i++)
  //       	l[i] = (double *)malloc(n * sizeof(double));

	// #pragma omp parallel for private(j)
	for(i=0;i<n;i++)
	{
		for (j = 0; j < n; j++)
		{
			if(i==j)
				l[i][j]=1;
			else
				l[i][j]=0;
		}
	}

	// double *temp_row;
	// temp_row = (double *)malloc((n)*sizeof(double));
  //
	// double *temp_col;
	// temp_col = (double *)malloc((n)*sizeof(double));

	int k_prime, k;

  // Pthread;

  pthread_t threads[no_threads];
  int z;
  for(z=0;z<no_threads;z++)
  {

      pthread_create(&threads[z], NULL,myThreadFun , &z);
  }

  for(int z=0;z<no_threads;z++)
  {
    pthread_join(threads[z], NULL);
  }

  //


	// t = clock() - t;
	// double time_taken = ((double)t)/CLOCKS_PER_SEC;
	// printf("Time taken: %f\n",time_taken);

	double *P[n];
	for (i=0; i<n; i++)
        	P[i] = (double *)malloc(n * sizeof(double));

	for (int i = 0; i < n; i++)
	{
		for(int j=0; j<n; j++)
		{
			if(pi[i]==j)
				P[i][j]=1;
			else
				P[i][j]=0;
		}
	}

	// double *left[n];
	// for (i=0; i<n; i++)
  //       	left[i] = (double *)malloc(n * sizeof(double));
	// matrix_mult(n,P,c,left);
	// double *right[n];
	// for (i=0; i<n; i++)
  //       	right[i] = (double *)malloc(n * sizeof(double));
	// matrix_mult(n,l,u,right);
	// double *res[n];
	// for (i=0; i<n; i++)
  //       	res[i] = (double *)malloc(n * sizeof(double));
	// matrix_sub(n,left,right,res);
  //
	// // print_matrix(n, res);
	// printf("Euclidean Norm: %f\n",euclidean_norm(n,res));
  //
	// //Free Memory
	// free(pi);
	// free(temp_col);
	// free(temp_row);
	// for (i=0; i<n; i++)
	// {
	// 	free(a[i]);
	// 	free(c[i]);
	// 	free(u[i]);
	// 	free(l[i]);
	// 	free(P[i]);
	// }
  for(i=0;i<n;i++)
  {
    for(j=0;j<n;j++)
    {
      printf("%lf ",l[i][j]);
    }

      printf("\n");
  }

  for(i=0;i<n;i++)
  {
    for(j=0;j<n;j++)
    {
      printf("%lf ",u[i][j]);
    }

    printf("\n");
  }

  for(i=0;i<n;i++)
  {
    for(j=0;j<n;j++)
    {
      printf("%lf ",P[i][j]);
    }

    printf("\n");
  }


	return 0;
}

void matrix_mult(int n, double *A[n], double *B[n],double *result[n])
{
	int i, j, k;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			result[i][j] = 0;
			for (k = 0; k < n; k++)
			{
				result[i][j] += A[i][k]*B[k][j];
			}
		}
	}
}

void matrix_sub(int n, double *A[n], double *B[n],double *result[n])
{
	int i, j;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			result[i][j]=A[i][j]-B[i][j];
		}
	}
}

void print_matrix(int n, double *A[n])
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
		    printf("%f\t", A[i][j]);
		}
		printf("\n");
	}
}

double euclidean_norm(int n, double *A[n])
{
	double result;

	for(int j=0; j<n; j++)
	{
		double temp=0;
		for(int i=0; i<n; i++)
		{
			temp += A[i][j]*A[i][j];
		}
		result += sqrt(temp);
	}

	return result;
}
