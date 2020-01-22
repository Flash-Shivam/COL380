#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>
#include <pthread.h>

#define ee 8000
int e;


int step = 0;

int inc;

int q; //dummy variable to check boundary condition .

double a[ee][ee],pi[ee],l[ee][ee],u[ee][ee],temp_col[ee],temp_row[ee];

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

    // printf("%d %d\n",start1,start2);
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
	scanf("%d",&n);
  e =  n;
	int no_threads;
	printf("Enter the no. of threads to be used: ");
	scanf("%d",&no_threads);
	printf("\n");

  inc = e/no_threads;
  q = no_threads;

	clock_t t;
	t=clock();

	double x;
  	struct drand48_data randBuffer;

  	srand48_r(time(NULL), &randBuffer);
  	int i,j;



	double *c[n];
	for (i=0; i<n; i++)
        	c[i] = (double *)malloc(n * sizeof(double));


	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
      x = 0;
      while(x==0){

				drand48_r(&randBuffer, &x);

				a[i][j] = 100 * x;
				c[i][j] = a[i][j];

      }

		}
	}



	for(i=0;i<n;i++)
	{
		pi[i]=i;
	}


	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			u[i][j]=0;
		}
	}

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


	t = clock() - t;
	double time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("Time taken: %f\n",time_taken);

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

  double left[n][n],right[n][n],res[n][n];
  int k;
  for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			left[i][j] = 0;
			for (k = 0; k < n; k++)
			{
				left[i][j] += P[i][k]*c[k][j];
			}
		}
	}

  for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			right[i][j] = 0;
			for (k = 0; k < n; k++)
			{
				right[i][j] += l[i][k]*u[k][j];
			}
		}
	}


  for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			res[i][j]=left[i][j]-right[i][j];
    //  printf("%lf ",res[i][j]);
		}
    // printf("\n");
	}


  double result;

	for(int j=0; j<n; j++)
	{
		double temp=0;
		for(int i=0; i<n; i++)
		{
			temp += res[i][j]*res[i][j];
		}
		result += sqrt(temp);
	}

  printf("Norm = %lf\n",result);

	return 0;
}
