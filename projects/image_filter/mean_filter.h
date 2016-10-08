#ifndef IMAGE_IMAGE_FILTER_MEAN_FILTER_H_
#define IMAGE_IMAGE_FILTER_MEAN_FILTER_H_
#include <assert.h>
#include <chrono>
#include <iostream>
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
class MeanFilter
{
public:
  MeanFilter() {}
  explicit MeanFilter(int radius) : radius_(radius) {}
  void set_radius(int radius) {
    assert(radius > 0);
    radius_ = radius;
  }
  void Filter(const Dtype* host_src, Dtype* host_dst, int width, int height);
private:
  int radius_;
  DISABLE_COPY_AND_ASSIGN(MeanFilter);
};
#endif  // !IMAGE_IMAGE_FILTER_MEAN_FILTER_H_
