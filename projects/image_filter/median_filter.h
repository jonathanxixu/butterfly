#ifndef IMAGE_IMAGE_FILTER_MEDIAN_FILTER_H_
#define IMAGE_IMAGE_FILTER_MEDIAN_FILTER_H_

#ifdef IMAGE_IMAGE_FILTER_MEDIAN_FILTER_H_
#define DLL_IMAGE_FILTER_MEDIAN_FILTER_API __declspec(dllexport)
#else
#define DLL_IMAGE_FILTER_MEDIAN_FILTER_API __declspec(dllimport)
#endif

#include <assert.h>
#include <chrono>

// Define macro min
#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

// Define max gray level.
#ifndef GRAY_LEVEL_MAX
#define GRAY_LEVEL_MAX 256
#endif

// Disable the copy and assignment operator for a class.
#define DISABLE_COPY_AND_ASSIGN(classname) \
private:\
  classname(const classname&);\
  classname& operator=(const classname&)


template<typename Dtype>
class DLL_IMAGE_FILTER_MEDIAN_FILTER_API MedianFilter
{
public:
  MedianFilter() : gate_(0.5) {}
  explicit MedianFilter(int radius) : radius_(radius), gate_(0.5) {}
  void set_radius(int radius) {
    assert(radius > 0);
    radius_ = radius;
  }
  void set_gate(float gate) {
    assert(gate > 0);
    assert(gate < 1);
    gate_ = gate;
  }
  void FilterByHistogram(const Dtype* host_src, Dtype* host_dst, int width, int height);
  void FilterByLocalSort(const Dtype* host_src, Dtype* host_dst, int width, int height);
private:
  int radius_;
  float gate_;
  DISABLE_COPY_AND_ASSIGN(MedianFilter);
};

class DLL_IMAGE_FILTER_MEDIAN_FILTER_API UcharMedianFilter
{
public:
  UcharMedianFilter() : gate_(0.5) {}
  explicit UcharMedianFilter(int radius) : radius_(radius), gate_(0.5) {}
  void set_radius(int radius) {
    assert(radius > 0);
    radius_ = radius;
  }
  void set_gate(float gate) {
    assert(gate > 0);
    assert(gate < 1);
    gate_ = gate;
  }
  void FilterByHistogram(const unsigned char* host_src, unsigned char* host_dst, int width, int height);
private:
  int radius_;
  float gate_;
  DISABLE_COPY_AND_ASSIGN(UcharMedianFilter);
};
#endif  // !IMAGE_IMAGE_FILTER_MEDIAN_FILTER_H_
