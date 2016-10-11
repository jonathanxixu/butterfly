project(image_filter)

set(CPPH_FILES
	median_filter.cpp
	median_filter.h
	mean_filter.cpp
	mean_filter.h
)
static_compile(image_filter ${CPPH_FILES})
file(GLOB_RECURSE SRCS_FILES *.cpp)
source_group("Source Files" FILES ${SRCS_FILES})

set (SOURCE_FILE ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/image_filter.lib)
set (DESTINATION_DIR ${CMAKE_SOURCE_DIR}/bin)
message(${SOURCE_FILE})
message(${DESTINATION_DIR})
add_custom_command(
    TARGET image_filter
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy ${SOURCE_FILE} ${DESTINATION_DIR}
)