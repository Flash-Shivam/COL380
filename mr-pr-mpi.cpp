#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <bits/stdc++.h>
#include <fstream>
using namespace std;


int main(int argc, char* argv[]){
  ifstream infile("bull.txt");
  int a,b;
  int number_of_pages = 1;
  while(infile >> a >> b)
  {
        number_of_pages = max(number_of_pages,max(a,b));
  }
  number_of_pages++;

  vector<int> adjacency_list[number_of_pages];

  ifstream infile1(argv[1]);
  int prev = 0;

  while(infile1 >> a >> b)
  {
      adjacency_list[a].push_back(b);
        //number_of_pages = max(number_of_pages,max(a,b));
  }

  double rank[number_of_pages];
  for(int i=0;i<number_of_pages;i++)
  {
    rank[i] = double(1)/number_of_pages;

  }
  //cout << number_of_pages << endl;
  int myRank;
  int size;
  int lower,upper;
  int elements_per_process;
  double alpha = 0.15;
  MPI_Status status;
    MPI_Init(&argc,&argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    elements_per_process = number_of_pages/size;

    for(int z=0;z<100;z++){
      double pagerank[number_of_pages];
      for(int i=0;i<number_of_pages;i++)
      {
        pagerank[i] = 0.0;
      }
      if(myRank==0)
      {
        for(int i=1;i<size;i++)
        {
          lower = i*elements_per_process;
          if(i == size-1)
          {
            upper = number_of_pages-1;
          }
          else{
            upper = i*elements_per_process+elements_per_process-1;
          }
          MPI_Send(&lower,1,MPI_INT,i,0,MPI_COMM_WORLD);
          MPI_Send(&upper,1,MPI_INT,i,0,MPI_COMM_WORLD);
        }
        //MPI_Barrier(MPI_COMM_WORLD);
        float lost_imp = 0.0;
        for(int i=0;i<elements_per_process;i++)
        {
          if(adjacency_list[i].size()==0)
          {
            lost_imp += rank[i];
          }
          else{
          double weight = rank[i]/adjacency_list[i].size();

          for(int j=0;j<adjacency_list[i].size();j++)
          {
            pagerank[adjacency_list[i][j]] += weight;
          }
        }
        }
        //MPI_Barrier(MPI_COMM_WORLD);
        double tmp[number_of_pages];

        float temp_imp =0.0;
        for (int i = 1; i <  size; i++) {
            MPI_Recv(&temp_imp, 1, MPI_DOUBLE,MPI_ANY_SOURCE, i,MPI_COMM_WORLD,&status);
            MPI_Recv(&tmp, number_of_pages, MPI_DOUBLE,MPI_ANY_SOURCE, 0,MPI_COMM_WORLD,&status);
            int sender = status.MPI_SOURCE;
            lost_imp += temp_imp;
            for(int j=0;j<number_of_pages;j++)
            {
              pagerank[j] += tmp[j];
            //  cout << pagerank[j] << " ";
            }

        }
      //  cout << lost_imp << " " << z << endl;
        for(int j=0;j<number_of_pages;j++)
        {
          rank[j] = alpha/number_of_pages + (1-alpha)*lost_imp/number_of_pages + (1-alpha)*pagerank[j];
          pagerank[j] = 0.0;
        //  cout << pagerank[j] << ": " ;
        }

        //cout << endl;

        // cout << endl;
          //MPI_Barrier(MPI_COMM_WORLD);
      }
      else{
        double r[number_of_pages];
        MPI_Recv(&lower,1,MPI_INT, 0, 0,MPI_COMM_WORLD,&status);
        MPI_Recv(&upper,1,MPI_INT, 0, 0,MPI_COMM_WORLD,&status);
        for(int i=0;i<number_of_pages;i++)
        {
          r[i] = 0.0;
        }
        float imp = 0.0;
          for(int i=lower;i<=upper;i++)
          {
            if(adjacency_list[i].size()==0)
            {
              imp += rank[i];
            }
            else{
            double weight = rank[i]/adjacency_list[i].size();

            for(int j=0;j<adjacency_list[i].size();j++)
            {
              r[adjacency_list[i][j]] += weight;
            }

          }
          }
          MPI_Send(&imp, 1, MPI_DOUBLE,0, myRank, MPI_COMM_WORLD);
          MPI_Send(&r, number_of_pages, MPI_DOUBLE,0, 0, MPI_COMM_WORLD);
        //  MPI_Barrier(MPI_COMM_WORLD);
      }

      MPI_Barrier(MPI_COMM_WORLD);

      MPI_Bcast(&rank,number_of_pages,MPI_DOUBLE,0,MPI_COMM_WORLD);


  }

  if(myRank==0)
  {
    //cout << "started" << endl;
    ofstream outFile(argv[3]);
    outFile<<std::fixed<<std::setprecision(15);
    float sum1 = 0.0;
    for(int k=0;k<number_of_pages;k++)
    {
      outFile << k << " = " << rank[k] << "\n";
     sum1 += rank[k];
    }
    outFile<<"s = "<<sum1<<"\n";
    outFile.close();
    //delete[] rank;
  //  cout << delta << " " << sum1 << endl;
  }

  MPI_Finalize();

}
