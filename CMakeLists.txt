project(youtubescope C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic")

include(FindPkgConfig)
pkg_search_module(SOUP libsoup-2.4)

if(NOT SOUP_FOUND)
    message(FATAL_ERROR "Libsoup not found.")
endif()

include_directories(${SOUP_INCLUDE_DIRS})

add_executable(yttest yttest.cpp YoutubeDownloader.cpp)
target_link_libraries(yttest ${SOUP_LDFLAGS})
