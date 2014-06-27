project(youtubescope C CXX)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic")

include(FindPkgConfig)
pkg_search_module(SOUP libsoup-2.4)
pkg_search_module(JSON jsoncpp)
pkg_search_module(ACC libaccounts-glib)

if(NOT SOUP_FOUND)
    message(FATAL_ERROR "Libsoup not found.")
endif()

if(NOT JSON_FOUND)
    message(FATAL_ERROR "Json-cpp not found.")
endif()

if(NOT ACC_FOUND)
    message(FATAL_ERROR "Libaccounts-glib not found.")
endif()

include_directories(${SOUP_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS})

add_library(ytlib STATIC YoutubeDownloader.cpp util.cpp)

add_executable(ytapp ytapp.cpp)
target_link_libraries(ytapp ytlib ${SOUP_LDFLAGS} ${JSON_LDFLAGS})

enable_testing()
add_executable(parsetest parsetest.cpp)
target_link_libraries(parsetest ytlib ${JSON_LDFLAGS})
add_test(parsetest parsetest)

include_directories(${ACC_INCLUDE_DIRS})
add_executable(accountstest accountstest.cpp)
target_link_libraries(accountstest ${ACC_LDFLAGS})
