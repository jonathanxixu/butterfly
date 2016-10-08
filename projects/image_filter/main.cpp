#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "image_filter/median_filter.h"
#include "image_filter/mean_filter.h"

#define TEST_TYPE float
#define LOG_FILE_CVS

#ifdef LOG_FILE_CVS
static std::ofstream g_file;
static char msg[512];
#define RECORD_INIT g_file.open("log.csv");
#define RECORD(level, ...) sprintf_s(msg, __VA_ARGS__); \
 g_file << msg << ",\n";
#define RECORD_END g_file.close();
#endif

#ifdef LOG_FILE_EASYLOGGING
#include "easylogging/easylogging++.h"
INITIALIZE_EASYLOGGINGPP
static char msg[512];
#define RECORD_INIT \
el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename, "test.log")
#define RECORD(level, ...) sprintf_s(msg, __VA_ARGS__); \
  LOG(level) << msg
#define RECORD_END
#endif

#ifdef LOG_PRINT
static char msg[512];
#define RECORD(level, ...) \
  sprintf_s(msg, __VA_ARGS__); \
  std::cout << #level <<": " <<msg << std::endl
#define RECORD_INIT
#define RECORD_END RECORD(INFO, "Finished, press ENTER to quit."); \
  getchar();
#endif

static char img_name[128];
#define SAVE_IMAGE(image, ...) \
  sprintf_s(img_name, __VA_ARGS__); \
  imwrite(img_name, image);


template<typename Dtype>
bool MedianFilterTestForNotUchar(cv::Mat img, int radius, int run_times) {
  // Read input image
  cv::Mat my_median, opencv_median;
  img.copyTo(my_median);
  int core_size = radius * 2 + 1;
  cv::medianBlur(img, opencv_median, core_size);

  // Fake data
  Dtype* fake_data_input = new Dtype[img.rows * img.cols];
  memset(fake_data_input, 0, sizeof(Dtype)*img.rows*img.cols);
  Dtype* fake_data_output = new Dtype[img.rows * img.cols];
  memset(fake_data_output, 0, sizeof(Dtype)*img.rows*img.cols);
  Dtype* fake_data_output2 = new Dtype[img.rows * img.cols];
  memset(fake_data_output2, 0, sizeof(Dtype)*img.rows*img.cols);
  for (int i = 0; i < img.rows; i++) {
    for (int j = 0; j < img.cols;j++) {
      fake_data_input[i*img.cols + j] =
        static_cast<Dtype>(img.data[i*img.cols + j] * 1.1f);
    }
  }

  // Compare images filtered by OPENCV and my filter
  MedianFilter<Dtype> median_filter;
  median_filter.set_radius(radius);
  median_filter.FilterByHistogram(fake_data_input, fake_data_output, img.cols, img.rows);
  median_filter.FilterByLocalSort(fake_data_input, fake_data_output2, img.cols, img.rows);
  
  double diff_sum = 0.0f, diff_sum2 = 0.0f;
  for (int i = radius; i < img.rows - radius; i++) {
    for (int j = radius; j < img.cols - radius; j++) {
      diff_sum += abs(round(fake_data_output[i*img.cols + j]/1.1f) -
        opencv_median.data[i*img.cols + j]);
      diff_sum2 += abs(round(fake_data_output2[i*img.cols + j] / 1.1f) -
        opencv_median.data[i*img.cols + j]);
    }
  }
  if (diff_sum < 0.1 && diff_sum2 < 0.1) {
    double my_median_time = 0.0, my_median_time2 = 0.0, opencv_median_time = 0.0;
    for (int i = 0; i < run_times; i++) {
      std::chrono::time_point<std::chrono::system_clock> start, end;
      start = std::chrono::system_clock::now();
      median_filter.FilterByHistogram(fake_data_input, fake_data_output, img.cols, img.rows);
      end = std::chrono::system_clock::now();
      my_median_time += std::chrono::duration<double>(end - start).count();

      start = std::chrono::system_clock::now();
      median_filter.FilterByLocalSort(fake_data_input, fake_data_output, img.cols, img.rows);
      end = std::chrono::system_clock::now();
      my_median_time2 += std::chrono::duration<double>(end - start).count();

      start = std::chrono::system_clock::now();
      cv::medianBlur(img, opencv_median, core_size);
      end = std::chrono::system_clock::now();
      opencv_median_time += std::chrono::duration<double>(end - start).count();
    }
    RECORD(INFO, "median filter, %12s, %2d * %2d, CORRECT, NO, %3d, , %10f, \
      %10f", typeid(Dtype).name(), core_size, core_size,
      run_times, my_median_time, my_median_time2);
    delete[] fake_data_input;
    delete[] fake_data_output;
    delete[] fake_data_output2;
  } else {
    RECORD(ERROR, "median filter,  %12s, %2d * %2d, WRONG, NO, %3d, , -, -",
      typeid(Dtype).name(), core_size, core_size, run_times);
    delete[] fake_data_input;
    delete[] fake_data_output;
    delete[] fake_data_output2;
    return false;
  }
  return true;
}
bool MedianFilterTestForUchar(cv::Mat img, int radius,
  bool need_save, int run_times) {
  // Read input image
  cv::Mat my_median, my_median2, my_median3, opencv_median;
  img.copyTo(my_median);
  img.copyTo(my_median2);
  img.copyTo(my_median3);

  // OPENCV as a standard
  int core_size = radius * 2 + 1;
  cv::medianBlur(img, opencv_median, core_size);

  // Compare images filtered by 4 methods
  UcharMedianFilter median_filter_uchar;
  MedianFilter<unsigned char> median_filter;
  median_filter_uchar.set_radius(radius);
  median_filter.set_radius(radius);

  // Test filter code is correct or not
  median_filter_uchar.FilterByHistogram(img.data, my_median.data, img.cols, img.rows);
  median_filter.FilterByHistogram(img.data, my_median2.data, img.cols, img.rows);
  median_filter.FilterByLocalSort(img.data, my_median3.data, img.cols, img.rows);
  double diff_sum = 0.0f, diff_sum2 = 0.0f, diff_sum3 = 0.0f;
  for (int i = radius; i < img.rows - radius; i++) {
    for (int j = radius; j < img.cols - radius; j++) {
      diff_sum += abs(my_median.data[i*img.cols + j] -
        opencv_median.data[i*img.cols + j]);
      diff_sum2 += abs(my_median2.data[i*img.cols + j] -
        opencv_median.data[i*img.cols + j]);
      diff_sum3 += abs(my_median3.data[i*img.cols + j] -
        opencv_median.data[i*img.cols + j]);
    }
  }
  
  // Calculate time
  if (diff_sum < 0.1 && diff_sum2 < 0.1 && diff_sum3 < 0.1) {
    if (need_save) {
      SAVE_IMAGE(my_median, "%d_my_median.bmp", core_size);
      SAVE_IMAGE(my_median2, "%d_my_median2.bmp", core_size);
      SAVE_IMAGE(my_median3, "%d_my_median3.bmp", core_size);
    }
    double my_median_time = 0.0f, my_median_time2 = 0.0f;
    double my_median_time3 = 0.0f, opencv_median_time = 0.0f;
    for (int i = 0; i < run_times; i++) {
      std::chrono::time_point<std::chrono::system_clock> start, end;

      start = std::chrono::system_clock::now();
      median_filter_uchar.FilterByHistogram(img.data, my_median.data, img.cols, img.rows);
      end = std::chrono::system_clock::now();
      my_median_time += std::chrono::duration<double>(end - start).count();

      start = std::chrono::system_clock::now();
      median_filter.FilterByHistogram(img.data, my_median.data, img.cols, img.rows);
      end = std::chrono::system_clock::now();
      my_median_time2 += std::chrono::duration<double>(end - start).count();

      start = std::chrono::system_clock::now();
      median_filter.FilterByLocalSort(img.data, my_median2.data, img.cols, img.rows);
      end = std::chrono::system_clock::now();
      my_median_time3 += std::chrono::duration<double>(end - start).count();

      start = std::chrono::system_clock::now();
      cv::medianBlur(img, opencv_median, core_size);
      end = std::chrono::system_clock::now();
      opencv_median_time += std::chrono::duration<double>(end - start).count();
    }
    RECORD(INFO, "median filter, unsigned char, %2d * %2d, CORRECT, YES, %3d, \
      %10f, %10f, %10f, %10f", core_size, core_size, run_times, my_median_time,
      my_median_time2, my_median_time3, opencv_median_time);
  } else {
    RECORD(ERROR, "median filter, unsigned char, %2d * %2d, WRONG, NO, -, -, -, -, -", 
      core_size, core_size);
    return false;
  }
  return true;
}
template<typename Dtype>
bool MeanFilterTestForNotUchar(cv::Mat img, int radius, int run_times) {
  // Read input image
  cv::Mat my_mean, opencv_mean;
  img.copyTo(my_mean);
  int core_size = radius * 2 + 1;
  cv::blur(img, opencv_mean, cv::Size(core_size, core_size), cv::Point(-1, -1));

  // Fake data
  Dtype* fake_data_input = new Dtype[img.rows * img.cols];
  memset(fake_data_input, 0, sizeof(Dtype)*img.rows*img.cols);
  Dtype* fake_data_output = new Dtype[img.rows * img.cols];
  memset(fake_data_output, 0, sizeof(Dtype)*img.rows*img.cols);
  for (int i = 0; i < img.rows; i++) {
    for (int j = 0; j < img.cols; j++) {
      fake_data_input[i*img.cols + j] =
        static_cast<Dtype>(img.data[i*img.cols + j]) * 1.1f;
    }
  }

  // Compare images filtered by OPENCV and my filter
  MeanFilter<Dtype> mean_filter;
  mean_filter.set_radius(radius);
  mean_filter.Filter(fake_data_input, fake_data_output, img.cols, img.rows);
  double diff_sum = 0.0f;
  for (int i = radius; i < img.rows - radius; i++) {
    for (int j = radius; j < img.cols - radius; j++) {
      diff_sum += abs(round(fake_data_output[i*img.cols + j] / 1.1f) -
        opencv_mean.data[i*img.cols + j]);
    }
  }
  if (diff_sum < 0.1) {
    double my_mean_time = 0.0, opencv_mean_time = 0.0;
    for (int i = 0; i < run_times; i++) {
      std::chrono::time_point<std::chrono::system_clock> start, end;
      start = std::chrono::system_clock::now();
      mean_filter.Filter(fake_data_input, fake_data_output, img.cols, img.rows);
      end = std::chrono::system_clock::now();
      my_mean_time += std::chrono::duration<double>(end - start).count();

      start = std::chrono::system_clock::now();
      cv::blur(img, opencv_mean, cv::Size(core_size, core_size), cv::Point(-1, -1));
      end = std::chrono::system_clock::now();
      opencv_mean_time += std::chrono::duration<double>(end - start).count();
    }
    RECORD(INFO, "mean filter, %12s, %2d * %2d, CORRECT, NO, %3d, %10f, %10f",
      typeid(Dtype).name(), core_size, core_size, run_times, my_mean_time,
      opencv_mean_time);
  } else {
    RECORD(ERROR, "median filter, %12s, %2d * %2d, WRONG, NO, -, -", 
      typeid(Dtype).name(), core_size, core_size);
    delete[] fake_data_input;
    delete[] fake_data_output;
    return false;
  }
  return true;
}
bool MeanFilterTestForUchar(cv::Mat img, int radius,
  bool need_save, int run_times) {
  // Read input image
  cv::Mat my_mean, opencv_mean;
  img.copyTo(my_mean);
  int core_size = radius * 2 + 1;
  cv::blur(img, opencv_mean, cv::Size(core_size, core_size), cv::Point(-1, -1));

  // Compare images filtered by OPENCV and my filter
  MeanFilter<unsigned char> mean_filter;
  mean_filter.set_radius(radius);
  mean_filter.Filter(img.data, my_mean.data, img.cols, img.rows);
  double diff_sum = 0.0f;
  for (int i = radius; i < img.rows - radius; i++) {
    for (int j = radius; j < img.cols - radius; j++) {
      diff_sum += abs(my_mean.data[i*img.cols + j] -
        opencv_mean.data[i*img.cols + j]);
    }
  }
  if (diff_sum < 0.1) {
    if (need_save) {
      SAVE_IMAGE(my_mean, "%d_my_mean.bmp", core_size);
      SAVE_IMAGE(opencv_mean, "%d_opencv_mean.bmp", core_size);
    }

    // Record loop filter time
    double my_mean_time = 0.0, opencv_mean_time = 0.0;
    for (int i = 0; i < run_times; i++) {
      std::chrono::time_point<std::chrono::system_clock> start, end;
      start = std::chrono::system_clock::now();
      mean_filter.Filter(img.data, my_mean.data, img.cols, img.rows);
      end = std::chrono::system_clock::now();
      my_mean_time += std::chrono::duration<double>(end - start).count();

      start = std::chrono::system_clock::now();
      cv::blur(img, opencv_mean, cv::Size(core_size, core_size), cv::Point(-1, -1));
      end = std::chrono::system_clock::now();
      opencv_mean_time += std::chrono::duration<double>(end - start).count();
    }
    RECORD(INFO, "mean filter, unsigned char, %2d * %2d, CORRECT, YES, %3d, \
      %10f, %10f", core_size, core_size, run_times, my_mean_time, 
      opencv_mean_time);
  } else {
    RECORD(ERROR, "median filter, unsigned char, %2d * %2d, WRONG, NO, -,  -, \
      -", core_size, core_size);
    return false;
  }
  return true;
}

int main(int argc, char *argv[]) {
  RECORD_INIT;
  // Input parameter check
  if (argc != 5) {
    RECORD(ERROR, "Usage: ImageProcessing.exe {image} \
          {radius array, e.g.[1, 3, 5, 9, 15, 21]} {run_times} \
          {save image:1, not:0(default)}");
    return 0;
  }
  assert(0 < atoi(argv[3]));
  assert(0 == atoi(argv[4]) || 1 == atoi(argv[4]));
  std::vector<int> radius_vec;
  std::string radius_str(argv[2]);
  std::string::iterator it = radius_str.begin();
  radius_str.erase(it + strlen(argv[2]) - 1);
  radius_str.erase(it);
  size_t pos = radius_str.find(',');
  while (radius_str.npos != pos) {
    std::string str = radius_str.substr(0, pos);
    assert(0 < atoi(str.c_str()));
    radius_vec.push_back(atoi(str.c_str()));
    radius_str.erase(0, pos + 1);
    pos = radius_str.find(',');
  }
  radius_vec.push_back(atoi(radius_str.c_str()));
  // Input image
  cv::Mat img = cv::imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
  RECORD(INFO, "Input image: height is %d and width is %d\r\n", img.rows, img.cols);
  //Test 1
  RECORD(INFO, "FileterName,ImageType,FilterSize,CorrectOrNot,SaveImage,LoopTimes,Method1Time,Method2Time,Method3Time,OpencvTime");
  for (int i = 0; i < radius_vec.size(); i++) {
    MedianFilterTestForUchar(img, radius_vec[i], atoi(argv[4]) > 0, atoi(argv[3]));
  }
  // Test 2
  RECORD(INFO, "");
  RECORD(INFO, "FileterName,ImageType,FilterSize,CorrectOrNot,SaveImage,LoopTimes,Method1Time,Method2Time,Method3Time");
  for (int i = 0; i < radius_vec.size(); i++) {
    MedianFilterTestForNotUchar<float>(img, radius_vec[i], atoi(argv[3])/2);
  }
  // Test 3
  //RECORD(INFO, "");
  //RECORD(INFO, "FileterName,ImageType,FilterSize,CorrectOrNot,SaveImage,LoopTimes,Method1Time,OpencvTime");
  //for (int i = 0; i < radius_vec.size(); i++) {
  //  MeanFilterTestForUchar(img, radius_vec[i], atoi(argv[4]) > 0, atoi(argv[3]));
  //}
  // Test 4
  //RECORD(INFO, "");
  //for (int i = 0; i < radius_vec.size(); i++) {
  //  MeanFilterTestForNotUchar<float>(img, radius_vec[i], atoi(argv[3]));
  //}
  RECORD_END;
  return 0;
}
