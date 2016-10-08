#include <new>
#include <memory>
#include "image_filter/mean_filter.h"

template class MeanFilter<unsigned char>;
template class MeanFilter<float>;
template class MeanFilter<double>;

/**
* Extend image edge by copying adjacent pixel
*/
template<typename Dtype>
void ExtendMatrixEdge(const Dtype *host_src, Dtype* host_extend_src,
  int width, int height, int radius) {
  int width_extend = width + radius * 2;
  for (int i = 0; i < height; i++) {
    memcpy_s(host_extend_src + width_extend * (i + radius) + radius,
      width* sizeof(Dtype), host_src + width * i, width* sizeof(Dtype));
    for (int j = 0; j < radius; j++) {
      host_extend_src[(i + radius) * width_extend + j] =
        host_src[i * width + radius - j];
      host_extend_src[(i + radius) * width_extend + width + radius + j] =
        host_src[(i + 1)*width - j];
    }
  }
  for (int i = 0; i < width + radius * 2; i++) {
    for (int j = 0; j < radius; j++) {
      host_extend_src[j * width_extend + i]
        = host_extend_src[(radius * 2 - j) * width_extend + i];
      host_extend_src[(height - 1 + 2 * radius - j) * (width + radius * 2) + i]
        = host_extend_src[(height - 1 + j) * width_extend + i];
    }
  }
}

/**
* Get initial sum when start from a new row.
*/
template<typename Dtype>
void GetInitSum(const Dtype *host_src, double* sum_cols,
  int radius, int width) {
  int core_size = radius * 2 + 1;
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < core_size; j++) {
      sum_cols[i] += host_src[i + width * j];
    }
  }
}

/**
* Update sum when filter core move towards right.
*/
template<typename Dtype>
void UpdateSum(const Dtype *host_src, double* sum_cols,
  int radius, int height_pos, int width) {
  for (int k = 0; k < width; k++) {
    sum_cols[k] -= host_src[k + width * (height_pos - radius)];
    sum_cols[k] += host_src[k + width * (height_pos + radius + 1)];
  }
}

/**
* Mean filtering helper, o(r) for unsigned char
*/
void MeanFilterHelper(const unsigned char *host_src, unsigned char *host_dst,
  int width, int height, int radius) {
  int core_size = radius * 2 + 1;
  double* sum_cols = nullptr;
  try {
    sum_cols = new double[width];
    memset(sum_cols, 0, sizeof(double) * width);
  }
  catch (std::bad_alloc) {
    exit(1);
  }
  GetInitSum(host_src, sum_cols, radius, width);
  int i = 0;
  for (int i = radius; i < height - radius - 1; i++) {
    for (int j = radius; j < width - radius; j++) {
      double sum = 0;
      for (int m = 0; m < core_size; m++) {
        sum += sum_cols[m + j - radius];
      }
      host_dst[j + i*width] =
        (unsigned char)(round(sum / (core_size * core_size)));
    }
    UpdateSum(host_src, sum_cols, radius, i, width);
  }
  for (int j = radius; j < width - radius; j++) {
    double sum = 0;
    for (int m = 0; m < core_size; m++) {
      sum += sum_cols[m + j - radius];
    }
    host_dst[j + i*width] =
      (unsigned char)(round(sum / (core_size * core_size)));
  }
  delete[] sum_cols;
}

/**
* Mean filtering helper, o(r)
*/
template<typename Dtype>
void MeanFilterHelper(const Dtype *host_src, Dtype *host_dst,
  int width, int height, int radius) {
  int core_size = radius * 2 + 1;
  double* sum_cols = nullptr;
  try {
    sum_cols = new double[width];
    memset(sum_cols, 0, sizeof(double) * width);
  }
  catch (std::bad_alloc) {
    exit(1);
  }
  GetInitSum(host_src, sum_cols, radius, width);
  int i = 0;
  for (int i = radius; i < height - radius - 1; i++) {
    for (int j = radius; j < width - radius; j++) {
      double sum = 0;
      for (int m = 0; m < core_size; m++) {
        sum += sum_cols[m + j - radius];
      }
      host_dst[j + i*width] = static_cast<Dtype>(sum / (core_size * core_size));
    }
    UpdateSum(host_src, sum_cols, radius, i, width);
  }
  for (int j = radius; j < width - radius; j++) {
    double sum = 0;
    for (int m = 0; m < core_size; m++) {
      sum += sum_cols[m + j - radius];
    }
    host_dst[j + i*width] = static_cast<Dtype>(sum / (core_size * core_size));
  }
  delete[] sum_cols;
}

/**
* Mean filtering.
* Extend image edge by copying adjacent pixel, then execute mean filtering.
*
* \param host_src   Source image data.
* \param host_dst   Destination image data. Must be preallocated.
* \param width			Source image and destination image width, in pixels.
* \param height			Source image and destination image height, in pixels.
* \param radius			Median filter radius. The kernelis a 2*r+1 by 2*r+1 square.
*/
template<typename Dtype>
void MeanFilter<Dtype>::Filter(const Dtype* host_src, Dtype* host_dst,
  int width, int height) {
  assert(nullptr != host_src);
  assert(nullptr != host_dst);
  assert(0 < width);
  assert(0 < height);
  assert(radius_ < MIN(width, height));

  Dtype* host_extend_src = nullptr;
  Dtype* host_extend_dst = nullptr;
  try {
    host_extend_src = new Dtype[(width + radius_ * 2)*(height + radius_ * 2)];
    host_extend_dst = new Dtype[(width + radius_ * 2)*(height + radius_ * 2)];
  }
  catch (std::bad_alloc) {
    exit(1);
  }
  ExtendMatrixEdge(host_src, host_extend_src, width, height, radius_);
  MeanFilterHelper(host_extend_src, host_extend_dst, width + 2 * radius_,
    height + 2 * radius_, radius_);
  for (int i = 0; i < height; i++) {
    memcpy_s(host_dst + i*width, width*sizeof(Dtype),
      host_extend_dst + (width + 2 * radius_)*(radius_ + i) + radius_,
      width * sizeof(Dtype));
  }
  delete[] host_extend_src;
  delete[] host_extend_dst;
}