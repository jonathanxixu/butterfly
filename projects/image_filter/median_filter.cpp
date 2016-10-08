#include <new>
#include <memory>
#include "image_filter/median_filter.h"

template class MedianFilter<unsigned char>;
template class MedianFilter<float>;
template class MedianFilter<double>;

// Quick sort
template<typename Dtype>
void QuickSort(Dtype* src, int l, int r) {
  int i = l, j = r;
  Dtype temp = src[l];
  while (i < j) {
    while (i < j && src[j] >= temp) { j--; }
    src[i] = src[j];
    while (i < j && src[i] <= temp) { i++; }
    src[j] = src[i];
  }
  src[i] = temp;
  if (l < i - 1) {
    QuickSort(src, l, i - 1);
  }
  if (r > i + 1) {
    QuickSort(src, i + 1, r);
  }
}

// Binary find
template<typename Dtype>
int BinaryFind(Dtype* src, int l, int r, Dtype v) {
  int mid = (l + r) / 2;
  if (src[mid] > v) {
    return BinaryFind(src, l, mid - 1, v);
  }
  if (src[mid] < v) {
    return BinaryFind(src, mid + 1, r, v);
  }
  return mid;
}

// Remove duplicates
template<typename Dtype>
int RemoveDuplicates(const Dtype* src, int len, Dtype* dst) {
  int j = 0;
  dst[0] = src[0];
  for (int i = 1; i < len; i++) {
    if (src[i] != dst[j]) {
      dst[++j] = src[i];
    }
  }
  return j + 1;
}

// Extend image edge by copying adjacent pixel
template<typename Dtype>
void ExtendMatrixEdge(const Dtype *host_src, Dtype* host_extend_src,
  int width, int height, int radius) {
  int width_extend = width + radius * 2;
  for (int i = 0; i < height; i++) {
    memcpy_s(host_extend_src + width_extend * (i + radius) + radius,
      width* sizeof(Dtype), host_src + width * i, width* sizeof(Dtype));
    for (int j = 0; j < radius; j++) {
      int row_extend = (i + radius) * width_extend;
      host_extend_src[row_extend + j] = host_src[i * width + radius - j];
      host_extend_src[row_extend + width + radius + j] =
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

// Get initial histogram array when start from a new row
template<typename Dtype>
void GetInitHist(const Dtype *host_src, int *his, int radius,
  int height_pos, int width_pos, int width) {
  int core_size = radius * 2 + 1;
  for (int i = 0; i < core_size; i++) {
    for (int j = 0; j < core_size; j++) {
      his[host_src[(height_pos + i - radius) * width + width_pos + j - radius]]++;
    }
  }
}

// Update histogram array when filter core move towards right.
template<typename Dtype>
void UpdateHist(const Dtype *host_src, int *his, int radius,
  int height_pos, int width_pos, int width) {
  int core_size = radius * 2 + 1;
  for (int i = 0; i < core_size; i++) {
    int some_row = (height_pos + i - radius) * width + width_pos;
    his[host_src[some_row - radius - 1]]--;
    his[host_src[some_row + radius]]++;
  }
}


// Count the histogram array, and return the value when trigger the gate
int GetHistMediumValue(int* his, int size, int radius, float gate) {
  int sum = 0, value = -1;
  int stop_point = static_cast<int>((radius * 2 + 1) * (radius * 2 + 1) * gate);
  for (int i = 0; i < size; i++) {
    sum += his[i];
    if (sum > stop_point) {
      value = i;
      break;
    }
  }
  return value;
}

// Median filtering Helper, unsigned char, o(N)
void GetMedianByHistogram(const unsigned char *host_src,
  unsigned char *host_dst, int width, int height, int radius, float gate) {
  int histogram[GRAY_LEVEL_MAX];
  for (int i = radius; i < height - radius; i++) {
    for (int j = radius; j < width - radius; j++) {
      if (j == radius) {
        memset(histogram, 0, GRAY_LEVEL_MAX * sizeof(int));
        GetInitHist(host_src, histogram, radius, i, j, width);
      } else {
        UpdateHist(host_src, histogram, radius, i, j, width);
      }
      host_dst[j + i*width] =
        GetHistMediumValue(histogram, GRAY_LEVEL_MAX, radius, gate);
    }
  }
}

// Median filtering Helper, others, o(N)
template<typename Dtype>
void GetMedianByHistogram(const Dtype *host_src, Dtype *host_dst,
  int width, int height, int radius, float gate) {
  // init
  Dtype* host_sort = nullptr;
  Dtype* host_unique = nullptr;
  int* host_ordinal = nullptr;

  host_sort = new Dtype[width * height];
  memcpy_s(host_sort, sizeof(Dtype) * width * height, host_src,
    sizeof(Dtype) * width * height);
  host_unique = new Dtype[width * height];
  memset(host_unique, 0, sizeof(Dtype) * width * height);
  host_ordinal = new int[width * height];
  memset(host_ordinal, 0, sizeof(int) * width * height);
  // sort input image value
  QuickSort(host_sort, 0, width*height - 1);
  // remove duplicate pixel
  int his_size = RemoveDuplicates(host_sort, width*height, host_unique);
  // ordinal transform
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      host_ordinal[i*width + j] =
        BinaryFind(host_unique, 0, his_size - 1, host_src[i*width + j]);
    }
  }
  // get median value by histogram
  int* extend_his = new int[his_size];
  for (int i = radius; i < height - radius; i++) {
    for (int j = radius; j < width - radius; j++) {
      if (j == radius) {
        memset(extend_his, 0, his_size * sizeof(int));
        GetInitHist(host_ordinal, extend_his, radius, i, j, width);
      } else {
        UpdateHist(host_ordinal, extend_his, radius, i, j, width);
      }
      host_dst[j + i*width] =
        host_unique[GetHistMediumValue(extend_his, his_size, radius, gate)];
    }
  }
  // source recovery
  delete[] host_sort;
  delete[] host_unique;
  delete[] host_ordinal;
  delete[] extend_his;
}


/**
* Median filtering for all types, specification template for unsigned char.
* Extend image edge by copying adjacent pixel, then execute median filtering.
* \param host_src   Source image data.
* \param host_dst   Destination image data. Must be preallocated.
* \param width			Image width, in pixels.
* \param height			Image height, in pixels.
* \param radius			Median filter radius. The kernel is a 2*r+1 by 2*r+1 square.
* \param gate				Filter gate, default value is 0.5.
*/
template<typename Dtype>
void MedianFilter<Dtype>::FilterByHistogram(const Dtype* host_src, Dtype* host_dst,
  int width, int height) {
  // Input check
  assert(nullptr != host_src);
  assert(nullptr != host_dst);
  assert(0 < width);
  assert(0 < height);
  assert(radius_ < MIN(width, height));
  Dtype* host_extend_src = nullptr;
  Dtype* host_extend_dst = nullptr;
  // Get memory
  int width_extend = width + radius_ * 2;
  int height_extend = height + radius_ * 2;
  host_extend_src = new Dtype[width_extend * height_extend];
  host_extend_dst = new Dtype[width_extend * height_extend];
  // Filter
  ExtendMatrixEdge(host_src, host_extend_src, width, height, radius_);
  GetMedianByHistogram(host_extend_src, host_extend_dst, width_extend,
    height_extend, radius_, gate_);
  for (int i = 0; i < height; i++) {
    memcpy_s(host_dst + i*width, width*sizeof(Dtype),
      host_extend_dst + width_extend * (radius_ + i) + radius_,
      width * sizeof(Dtype));
  }
  // Resource recovery
  delete[] host_extend_src;
  delete[] host_extend_dst;
}


// Add source data to buffer
template<typename Dtype>
void SaveBuffer(const Dtype *host_src, Dtype *buffer, int radius,
  int height_pos, int width_pos, int width) {
  int core_size = radius * 2 + 1;
  for (int i = 0; i < core_size; i++) {
    int some_row = (height_pos + i - radius) * width + width_pos;
    for (int j = 0; j < core_size; j++) {
      buffer[i * core_size + j] = host_src[some_row + j - radius];
    }
  }
}

// Replace a data in a sorted array to a new value, and then resort it
template<typename Dtype>
void ReplaceSortedBuffer(Dtype* buffer, int pos, int radius, Dtype new_val)
{
  int core_size = radius * 2 + 1;
  buffer[pos] = new_val;
  for (int i = pos - 1; i >= 0; i--) {
    if (buffer[i] > new_val) {
      buffer[i + 1] = buffer[i];
      buffer[i] = new_val;
    } else {
      break;
    }
  }
  for (int i = pos + 1; i < core_size * core_size; i++) {
    if (buffer[i] < new_val) {
      buffer[i - 1] = buffer[i];
      buffer[i] = new_val;
    } else {
      break;
    }
  }
}

// Get median value by sort the local buffer
template<typename Dtype>
void GetMedianByLocalSort(const Dtype*host_src, Dtype *host_dst,
  int width, int height, int radius, float gate) {
  int core_size = radius * 2 + 1;
  int wnd_size = core_size * core_size;
  int get_size = static_cast<int>(wnd_size * gate);
  Dtype* buffer = new Dtype[core_size * core_size];
  for (int i = radius; i < height - radius; i++) {
    for (int j = radius; j < width - radius; j++) {
      if (j == radius) {
        memset(buffer, 0, sizeof(Dtype)*wnd_size);
        SaveBuffer(host_src, buffer, radius, i, j, width);
        QuickSort(buffer, 0, wnd_size - 1);
      } else {
        for (int m = 0; m < core_size; m++) {
          int pos = BinaryFind(buffer, 0, wnd_size,
            host_src[(i - radius + m) * width + j - radius - 1]);
          ReplaceSortedBuffer(buffer, pos, radius,
            host_src[(i - radius + m) * width + j + radius]);
        }
      }
      host_dst[j + i*width] = buffer[get_size];
    }
  }
  delete[] buffer;
}

/**
* Median filtering for all types.
* Extend image edge by copying adjacent pixel, then execute median filtering.
* \param host_src   Source image data.
* \param host_dst   Destination image data. Must be preallocated.
* \param width			Image width, in pixels.
* \param height			Image height, in pixels.
* \param radius			Median filter radius. The kernel is a 2*r+1 by 2*r+1 square.
* \param gate				Filter gate, default value is 0.5.
*/
template<typename Dtype>
void MedianFilter<Dtype>::FilterByLocalSort(const Dtype* host_src, Dtype* host_dst,
  int width, int height) {
  // Input check
  assert(nullptr != host_src);
  assert(nullptr != host_dst);
  assert(0 < width);
  assert(0 < height);
  assert(radius_ < MIN(width, height));
  Dtype* host_extend_src = nullptr;
  Dtype* host_extend_dst = nullptr;
  // Get memory
  int width_extend = width + radius_ * 2;
  int height_extend = height + radius_ * 2;
  host_extend_src = new Dtype[width_extend * height_extend];
  host_extend_dst = new Dtype[width_extend * height_extend];
  // Filter
  ExtendMatrixEdge(host_src, host_extend_src, width, height, radius_);
  GetMedianByLocalSort(host_extend_src, host_extend_dst, width + 2 * radius_,
    height + 2 * radius_, radius_, gate_);
  for (int i = 0; i < height; i++) {
    memcpy_s(host_dst + i*width, width*sizeof(Dtype),
      host_extend_dst + width_extend * (radius_ + i) + radius_,
      width * sizeof(Dtype));
  }
  // Resource recovery
  delete[] host_extend_src;
  delete[] host_extend_dst;
}

// Calculate the sum of the histograms
void GetSumsOfHist(int* his, int** his_col,
  int start, int nums) {
  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < nums; j++) {
      his[i] += his_col[start + j][i];
    }
  }
}

// Add a histogram, then sub another
void AddSubHist(int* his, int* his_add, int* his_sub) {
  for (int i = 0; i < 256; i++) {
    his[i] += his_add[i];
    his[i] -= his_sub[i];
  }
}

// Update some histogram in array with the movement of filter window
void UpdateHistInArray(int** his_col, const unsigned char* host_src,
  int width, int new_width, int new_height, int radius) {
  unsigned char delpoint =
    host_src[(new_height - radius - 1) * width + new_width + radius];
  unsigned char addpoint =
    host_src[(new_height + radius) * width + new_width + radius];
  his_col[new_width + radius][delpoint]--;
  his_col[new_width + radius][addpoint]++;
}

// Update some histograms in array from 0, the size is the size of filter window
void UpdateHistFromZeroToCoreSize(int** his_col,
  const unsigned char* host_src, int width,
  int new_height_pos, int radius) {
  for (int i = 0; i < radius * 2 + 1; i++) {
    his_col[i][host_src[(new_height_pos - radius - 1) * width + i]]--;
    his_col[i][host_src[(new_height_pos + radius) * width + i]]++;
  }
}

// Median filter helper for unsigned char, o(1)
void GetUcharMedianByHistogram(const unsigned char *host_src,
  unsigned char *host_dst, int width, int height,
  int radius, float gate) {
  // Init a histogram and a histogram array
  int core_size = radius * 2 + 1;
  int histogram[GRAY_LEVEL_MAX];
  memset(histogram, 0, sizeof(int) * GRAY_LEVEL_MAX);
  int** his_cols = nullptr;
  his_cols = new int*[width];
  for (int i = 0; i < width; i++) {
    his_cols[i] = new int[GRAY_LEVEL_MAX];
    memset(his_cols[i], 0, sizeof(int) * GRAY_LEVEL_MAX);
  }
  // Histogram array assignment
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < core_size; j++) {
      his_cols[i][host_src[i + j*width]]++;
    }
  }
  // Calculate the histogram of first pixel in first row,
  // then calculate medium value
  GetSumsOfHist(histogram, his_cols, 0, core_size);
  host_dst[radius + radius*width] =
    GetHistMediumValue(histogram, GRAY_LEVEL_MAX, radius, gate);
  // Calculate the histogram of the other pixel in first row,
  // then calculate medium value
  for (int i = radius + 1; i < width - radius; i++) {
    AddSubHist(histogram, his_cols[i + radius], his_cols[i - radius - 1]);
    host_dst[i + radius*width] =
      GetHistMediumValue(histogram, GRAY_LEVEL_MAX, radius, gate);
  }
  // Calculate medium value in other row
  for (int j = radius + 1; j < height - radius; j++) {
    // Update the first filter core size cols in histogram array
    UpdateHistFromZeroToCoreSize(his_cols, host_src, width, j, radius);
    // Calculate the histogram of first pixel in row,
    // then calculate medium value
    memset(histogram, 0, sizeof(int) * GRAY_LEVEL_MAX);
    GetSumsOfHist(histogram, his_cols, 0, core_size);
    host_dst[radius + j * width] =
      GetHistMediumValue(histogram, GRAY_LEVEL_MAX, radius, gate);
    // Calculate the histogram of the other pixel in row
    for (int i = radius + 1; i < width - radius; i++) {
      // Update col in histogram array
      // then calculate the histogram with the movement of the filter window
      UpdateHistInArray(his_cols, host_src, width, i, j, radius);
      AddSubHist(histogram, his_cols[i + radius], his_cols[i - radius - 1]);
      host_dst[i + j * width] =
        GetHistMediumValue(histogram, GRAY_LEVEL_MAX, radius, gate);
    }
  }
  // Resource recovery
  for (int i = 0; i < width; i++) {
    delete[] his_cols[i];
  }
  delete[] his_cols;
}

/**
* Median filtering.
* Extend image edge by copying adjacent pixel, then execute median filtering.
*
* \param host_src   Source image data.
* \param host_dst   Destination image data. Must be preallocated.
* \param width			Image width, in pixels.
* \param height			Image height, in pixels.
* \param radius			Median filter radius. The kernel is a 2*r+1 by 2*r+1 square.
* \param gate				Filter gate, default value is 0.5.
*/
void UcharMedianFilter::FilterByHistogram(const unsigned char* host_src,
  unsigned char* host_dst, int width, int height) {
  // Input check
  assert(nullptr != host_src);
  assert(nullptr != host_dst);
  assert(0 < width);
  assert(0 < height);
  assert(radius_ < MIN(width, height));
  unsigned char* host_extend_src = nullptr;
  unsigned char* host_extend_dst = nullptr;
  // Get memory
  int width_extend = width + radius_ * 2;
  int height_extend = height + radius_ * 2;
  host_extend_src = new unsigned char[width_extend * height_extend];
  host_extend_dst = new unsigned char[width_extend * height_extend];
  // Filter
  ExtendMatrixEdge(host_src, host_extend_src, width, height, radius_);
  GetUcharMedianByHistogram(host_extend_src, host_extend_dst, width_extend,
    height_extend, radius_, gate_);
  for (int i = 0; i < height; i++) {
    memcpy_s(host_dst + i*width, width*sizeof(unsigned char),
      host_extend_dst + width_extend * (radius_ + i) + radius_,
      width * sizeof(unsigned char));
  }
  // Resource recovery
  delete[] host_extend_src;
  delete[] host_extend_dst;
}