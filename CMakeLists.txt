cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)
PROJECT(firefly_rkmedia_demo)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

#set(CMAKE_CXX_FLAGS "-Wall")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fnon-call-exceptions")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#find_package(PkgConfig)

include(FindPkgConfig)
pkg_check_modules(LIBLIVEMEDIA live555)

#find_package(LiveMedia REQUIRED)
if(LIBLIVEMEDIA_FOUND)
  message(STATUS "${LIBLIVEMEDIA_LIBDIR}")
  message(STATUS "${LIBLIVEMEDIA_LIBRARIES}")

  #add_definitions(-DLIBLIVEMEDIA)
  message(STATUS "${LIBLIVEMEDIA_INCLUDE_DIRS}")
  include_directories(${LIBLIVEMEDIA_INCLUDE_DIRS})
endif()


include_directories(
    #/usr/include/c++/8
    #/usr/include/arm-linux-gnueabihf/c++/8
    ${CMAKE_SOURCE_DIR}/rtspclient
)

link_directories(${LIBLIVEMEDIA_LIBDIR})

set(LIVE555_LIBS
    ${LIBLIVEMEDIA_LIBRARIES}
)
#message(FATAL_ERROR "${LIVE555_LIBS}")
set(RTSPCLIENT_LIB
    ${CMAKE_SOURCE_DIR}/build/rtspclient/librtspclient.so
)


###################################
add_subdirectory(rtspclient)
###################################

add_executable(rtsptest ${CMAKE_SOURCE_DIR}/rtsptest.cc)
target_link_libraries(rtsptest ${LIVE555_LIBS} ${RTSPCLIENT_LIB} pthread)

