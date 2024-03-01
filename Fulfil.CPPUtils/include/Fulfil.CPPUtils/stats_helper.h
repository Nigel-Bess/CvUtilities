#ifndef STATS_HELPER_H_
#define STATS_HELPER_H_
#include <memory>
#include <iostream>
#include <sstream>
#include <vector>

namespace fulfil
{
namespace utils
{
template <class Unit> class StatsHelperDelegate
{
 public:
  virtual Unit add(Unit a, Unit b)=0;
  virtual Unit divide(Unit a, int b)=0;
  virtual Unit power(Unit a, double exponent)=0;
  virtual Unit subtract(Unit a, Unit b)=0;
  virtual Unit sum_base()=0;
  virtual Unit sqrt(Unit a)=0;
  virtual Unit max(Unit a, Unit b)=0;
  virtual Unit min(Unit a, Unit b)=0;
};

template <class Unit> class MeanStatistics
{
 public:
  Unit mean;
  Unit min;
  Unit max;
  MeanStatistics(Unit mean, Unit min, Unit max)
  {
    this->mean = mean;
    this->min = min;
    this->max = max;
  }
};

template <class Unit> class Statistics
{
 public:
  Unit mean;
  Unit min;
  Unit max;
  Unit stddev;
  Unit variance;
  Unit range;
  int sample_size;
  Statistics(std::shared_ptr<MeanStatistics<Unit>> mean_stats, Unit stddev, Unit variance, Unit range, int sample_size)
  {
    this->mean = mean_stats->mean;
    this->min = mean_stats->min;
    this->max = mean_stats->max;
    this->stddev = stddev;
    this->variance = variance;
    this->range = range;
    this->sample_size = sample_size;
  }
  void print()
  {
    std::cout << "Statistics:" << std::endl;
    std::cout << "Mean: " << this->mean << std::endl;
    std::cout << "Min: " << this->min << std::endl;
    std::cout << "Max: " << this->max << std::endl;
    std::cout << "Range: " << this->range << std::endl;
    std::cout << "Standard Deviation: " << this->stddev << std::endl;
    std::cout << "Variance: " << this->variance << std::endl;
    std::cout << "Sample Size: " << this->sample_size << std::endl << std::endl;
  }

  std::shared_ptr<std::string> asString()
  {
    std::ostringstream os;
    os << "Statistics:" << std::endl;
    os << "Mean: " << this->mean << std::endl;
    os << "Min: " << this->min << std::endl;
    os << "Max: " << this->max << std::endl;
    os << "Range: " << this->range << std::endl;
    os << "Standard Deviation: " << this->stddev << std::endl;
    os << "Variance: " << this->variance << std::endl;
    os << "Sample Size: " << this->sample_size << std::endl << std::endl;
    return std::make_shared<std::string>(os.str());
  }

  std::shared_ptr<std::string> asCsvRow()
  {
    std::ostringstream os;
    os << this->mean << ",";
    os << this->min << ",";;
    os << this->max << ",";
    os << this->range << ",";
    os << this->stddev << ",";
    os << this->variance << ",";
    os << this->sample_size;
    return std::make_shared<std::string>(os.str());
  }
};

template <class Unit> class StatsHelper
{
 private:
  std::shared_ptr<MeanStatistics<Unit>> iterate_for_mean(std::shared_ptr<std::vector<Unit>> samples)
  {
    if(samples->size() == 0)
    {
      throw std::invalid_argument("invalid sample set");
    }

    if(this->delegate.expired())
    {
      throw std::runtime_error("invalid delegate");
    }
    std::shared_ptr<StatsHelperDelegate<Unit>> tmp_delegate = this->delegate.lock();
    Unit sum = tmp_delegate->sum_base();
    Unit min = samples->at(0);
    Unit max = samples->at(0);
    for(int i = 0; i < samples->size(); i++)
    {
      sum = tmp_delegate->add(sum, samples->at(i));
      min = tmp_delegate->min(min, samples->at(i));
      max = tmp_delegate->max(max, samples->at(i));
    }
    return std::make_shared<MeanStatistics<Unit>>(tmp_delegate->divide(sum, samples->size()), min, max);
  }
 public:
  std::weak_ptr<StatsHelperDelegate<Unit>> delegate;

  Unit calculate_mean(std::shared_ptr<std::vector<Unit>> samples)
  {
    return this->iterate_for_mean(samples)->mean;
  }

  Unit calculate_variance(std::shared_ptr<std::vector<Unit>> samples)
  {
    Unit mean = this->calculate_mean(samples);
    return this->calculate_variance(samples, mean);
  }

  Unit calculate_variance(std::shared_ptr<std::vector<Unit>> samples, Unit mean)
  {
    if(samples->size() == 0)
    {
      throw std::invalid_argument("invalid sample set");
    }

    if(this->delegate.expired())
    {
      throw std::runtime_error("invalid delegate");
    }
    std::shared_ptr<StatsHelperDelegate<Unit>> tmp_delegate = this->delegate.lock();
    Unit variance = tmp_delegate->sum_base();
    for(int i = 0; i < samples->size(); i++)
    {
      Unit mean_diff = tmp_delegate->subtract(samples->at(i), mean);
      Unit square_diff = tmp_delegate->power(mean_diff, 2);
      variance = tmp_delegate->add(variance, square_diff);
    }
    return variance;
  }

  Unit calculate_std_dev(std::shared_ptr<std::vector<Unit>> samples)
  {
    Unit mean = this->calculate_mean(samples);
    return this->calculate_std_dev(samples, mean);
  }

  Unit calculate_std_dev(std::shared_ptr<std::vector<Unit>> samples, Unit mean)
  {
    if(samples->size() == 0)
    {
      throw std::invalid_argument("invalid sample set");
    }

    if(this->delegate.expired())
    {
      throw std::runtime_error("invalid delegate");
    }
    std::shared_ptr<StatsHelperDelegate<Unit>> tmp_delegate = this->delegate.lock();
    Unit var = this->calculate_variance(samples, mean);
    return tmp_delegate->sqrt(var);
  }

  std::shared_ptr<Statistics<Unit>> calculate_statistics(std::shared_ptr<std::vector<Unit>> samples)
  {
    std::shared_ptr<MeanStatistics<Unit>> mean_stats = this->iterate_for_mean(samples);
    Unit variance = this->calculate_variance(samples, mean_stats->mean);
    if(this->delegate.expired())
    {
      throw std::runtime_error("invalid delegate");
    }
    std::shared_ptr<StatsHelperDelegate<Unit>> tmp_delegate = this->delegate.lock();
    Unit stddev = tmp_delegate->sqrt(variance);
    return std::make_shared<Statistics<Unit>>(mean_stats, stddev, variance, tmp_delegate->subtract(mean_stats->max, mean_stats->min), samples->size());
  }
};
}
}
#endif