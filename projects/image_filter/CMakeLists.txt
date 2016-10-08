project(ImageFilter)

set(OpenCV_STATIC FALSE)
find_package(OpenCV REQUIRED
  HINTS $ENV{OPENCV})
if(${OpenCV_FOUND})
	include_directories(${OpenCV_INCLUDE_DIRS})
	message("${OpenCV_INCLUDE_DIRS}")
	set(CPPH_FILES
		main.cpp
		median_filter.cpp
		median_filter.h
		mean_filter.cpp
		mean_filter.h
	)
	execute_compile(ImageFilter ${CPPH_FILES})
	file(GLOB_RECURSE SRCS_FILES *.cpp)
	source_group("Source Files" FILES ${SRCS_FILES})
	target_link_libraries(ImageFilter ${OpenCV_LIBS})
else()
	message("ERROR: CAN NOT FIND OPENCV.")
	return()
endif()