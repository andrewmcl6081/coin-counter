project (program2)
cmake_minimum_required(VERSION 3.15)

find_package(OpenCV REQUIRED)

set(SOURCES program2.cpp)
add_executable(program2 ${SOURCES})
target_link_libraries(program2 ${OpenCV_LIBS})
