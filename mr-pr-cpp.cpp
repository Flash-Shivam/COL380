// Copyright (c) 2009-2016 Craig Henderson
// https://github.com/cdmh/mapreduce

#include <boost/config.hpp>
#include <typeinfo>
#if defined(BOOST_MSVC)
#   pragma warning(disable: 4127)

// turn off checked iterators to avoid performance hit
#   if !defined(__SGI_STL_PORT)  &&  !defined(_DEBUG)
#       define _SECURE_SCL 0
#       define _HAS_ITERATOR_DEBUGGING 0
#   endif
#endif

#include "mapreduce.hpp"

namespace pagerank {

    std::vector< std::vector<int> > adjacency_list;
    std::vector<float> pagerank_val;
    float alpha = 0.15;
    float lost_importance = 0.0;
    int number_of_pages = 1;
    void read_data(std::string filename, std::vector< std::vector<int> > &adjacency_list)
    {
      std::ifstream infile(filename);
      int a,b;

      while(infile >> a >> b)
      {
            number_of_pages = std::max(number_of_pages,std::max(a,b));
      }
      number_of_pages++;
      //std::vector<int> adjacency_list[number_of_pages];
      std::vector<int> pages;
      adjacency_list.insert(adjacency_list.begin(), number_of_pages, pages);
      std::ifstream infile1(filename);

      while(infile1 >> a >> b)
      {
          adjacency_list[a].push_back(b);
      }
      float val = 1.0/number_of_pages;
      // intialising the pagerank values
      for(int i=0;i<number_of_pages;i++)
      {
        pagerank_val.push_back(val);
      }

    }




template<typename MapTask>
class datasource : mapreduce::detail::noncopyable
{
  public:
    datasource() : sequence_(0)
    {

    }

    bool const setup_key(typename MapTask::key_type &key)
    {
        key = sequence_++;
        //std::cout << key << std::endl;
        return key < number_of_pages;
    }

    bool const get_data(typename MapTask::key_type const &key, typename MapTask::value_type &value)
    {
        value = pagerank_val[key];
        //std::cout << key << std::endl;
        return true;
    }

  private:
    unsigned sequence_;
};

struct map_task1 : public mapreduce::map_task<int , float>
{
    template<typename Runtime>
    void operator()(Runtime &runtime, key_type const &key, value_type const &value) const
    {
        if(adjacency_list[key].size()==0)
        {
          runtime.emit_intermediate(-1, value);
        }

      //  std::cout << key << std::endl;
    }
};

struct reduce_task1 : public mapreduce::reduce_task<int, float>
{
    template<typename Runtime, typename It>
    void operator()(Runtime &runtime, key_type const &key, It it, It ite) const
    {
        assert(key ==  -1);
        lost_importance = 0;
        while(it!=ite)
        {
            lost_importance += *it;
            it++;
        }
    }
};

typedef
mapreduce::job<pagerank::map_task1,
               pagerank::reduce_task1,
               mapreduce::null_combiner,
               pagerank::datasource<pagerank::map_task1>
> job1;

struct map_task2 : public mapreduce::map_task<int , float>
{
    template<typename Runtime>
    void operator()(Runtime &runtime, key_type const &key, value_type const &value) const
    {

      runtime.emit_intermediate(key, 0);
      int total_links = adjacency_list[key].size();
      for(int i=0;i<total_links;i++)
      {
          value_type value = pagerank_val[key]/total_links;
          runtime.emit_intermediate(adjacency_list[key][i], value);
      }

      //  std::cout << key << std::endl;
    }
};

struct reduce_task2 : public mapreduce::reduce_task<int, float>
{
    template<typename Runtime, typename It>
    void operator()(Runtime &runtime, key_type const &key, It it, It ite) const
    {
      float common_val = (alpha)/number_of_pages + (1.0-alpha)*lost_importance/number_of_pages;

      for (; it!=ite; ++it)
        common_val+=(1.0-alpha)*(*it);

      runtime.emit(key,fabs(common_val - pagerank_val[key]));

      pagerank_val[key] = common_val;
    }
};

typedef
mapreduce::job<pagerank::map_task2,
               pagerank::reduce_task2,
               mapreduce::null_combiner,
               pagerank::datasource<pagerank::map_task2>
> job2;

} // namespace friend_graph

int main(int argc, char *argv[])
{
  pagerank::read_data(argv[1], pagerank::adjacency_list);

  mapreduce::specification spec;
  int number_of_tasks;
  if (argc <=4)
    number_of_tasks = std::thread::hardware_concurrency();
  else
    number_of_tasks = std::atoi(argv[4]);

  spec.map_tasks = number_of_tasks;
  spec.reduce_tasks = number_of_tasks;

  auto start = std::chrono::high_resolution_clock::now();
  float epsilon = 1e-5;
  int max_loops = 100;

  for(int iter = 0;iter<max_loops;iter++)
  {

      mapreduce::results result1, result2;

      pagerank::job1::datasource_type ds1;
      pagerank::job1 job1(ds1, spec);

      job1.run<mapreduce::schedule_policy::cpu_parallel<pagerank::job1> >(result1);

      pagerank::job2::datasource_type ds2;
      pagerank::job2 job2(ds2, spec);


      job2.run<mapreduce::schedule_policy::cpu_parallel<pagerank::job2> >(result2);

      float error = 0;
      for (auto it=job2.begin_results(); it!=job2.end_results(); ++it)
          error+=it->second;

      std::cout <<  error << std::endl;

      if(error<epsilon)
      {
          break;
      }
  }

  auto end = std::chrono::high_resolution_clock::now();
  double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

  std::ofstream outFile(argv[3]);
  outFile<<std::fixed<<std::setprecision(15);
  float sum = 0.0;

  for(int i=0;i<pagerank::number_of_pages;i++)
      {
          outFile << i << " = " << pagerank::pagerank_val[i] << std::endl;
          sum += pagerank::pagerank_val[i];
      }

      outFile<<"s = "<<sum<<"\n";
      outFile.close();

  std::cout<<"mr-pr-cpp.cpp "<<argv[1]<<" "<<spec.reduce_tasks<<" "<<time_taken*1e-9<<std::endl;


    return 0;
}
