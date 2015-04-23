cmake_minimum_required(VERSION 2.8.7)
project (LOG_ROTATE)


set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
  
find_package(Boost COMPONENTS system filesystem regex REQUIRED)
if(NOT Boost_FOUND)
	die("Could not find Boost libraries, please make sure you have installed Boost")
endif()

include_directories( ${Boost_INCLUDE_DIR} )

set(WARNINGS "-Wall -Wextra -Wpointer-arith -Wundef -Wvla -Wwrite-strings -Wno-error=extra -Wno-error=deprecated-declarations -Wno-error=sign-compare -Wno-error=strict-aliasing -Wno-error=type-limits -Wno-unused-parameter -Wno-error=unused-variable -Wno-error=undef -Wno-error=uninitialized")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} -std=c++11")


add_executable(log_rotate src/main.cpp src/cLogRotate.cpp src/cLogRotate.h)

target_link_libraries(log_rotate
	${Boost_LIBRARIES})