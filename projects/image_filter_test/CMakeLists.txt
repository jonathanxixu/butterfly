project(image_filter_test)

set(OpenCV_STATIC FALSE)
find_package(OpenCV REQUIRED
  HINTS $ENV{OPENCV})
if(${OpenCV_FOUND})
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:100000000")
	include_directories(${OpenCV_INCLUDE_DIRS})
	message("${OpenCV_INCLUDE_DIRS}")
	set(CPPH_FILES main.cpp)
	execute_compile(image_filter_test ${CPPH_FILES})
	file(GLOB_RECURSE SRCS_FILES *.cpp)
	source_group("Source Files" FILES ${SRCS_FILES})
	target_link_libraries(image_filter_test ${OpenCV_LIBS})
	target_link_libraries(image_filter_test image_filter.lib)
	
	set (SOURCE_FILE ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/image_filter_test.exe)
	set (DESTINATION_DIR ${CMAKE_SOURCE_DIR}/bin)
	add_custom_command(
		TARGET image_filter_test
		POST_BUILD
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy ${SOURCE_FILE} ${DESTINATION_DIR}
	)
else()
	message("ERROR: CAN NOT FIND OPENCV.")
	return()
endif()