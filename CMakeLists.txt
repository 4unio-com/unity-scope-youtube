project(unity-scope-youtube CXX)
cmake_minimum_required(VERSION 2.8.10)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

set(cxx_version_required 4.9)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (NOT CMAKE_CXX_COMPILER_VERSION MATCHES "^${cxx_version_required}")
        message(FATAL_ERROR "g++ version must be ${cxx_version_required}!")
    endif()
endif()

add_definitions(
  -fno-permissive
  -std=c++11
  -pedantic
  -Wall
  -Wextra
  -fPIC
)

include(GNUInstallDirs)
find_package(PkgConfig)

find_package(
  Boost
  COMPONENTS
    iostreams
  REQUIRED
)

pkg_check_modules(
  SCOPE
  glib-2.0
  gio-unix-2.0
  libaccounts-glib
  libsignon-glib
  libunity-scopes>=0.5.0
  jsoncpp
  net-cpp
  process-cpp
  REQUIRED
)

include_directories(
  "${CMAKE_SOURCE_DIR}/include"
  ${Boost_INCLUDE_DIRS}
  ${SCOPE_INCLUDE_DIRS}
)

set(SCOPE_NAME unity-scope-youtube)
if(CLICK_MODE)
  add_subdirectory(click)
  set(SCOPE_INSTALL_NAME "com.pete-woods.youtube_youtube")
  set(SCOPE_INSTALL_DIR "click/youtube")
else()
  set(SCOPE_INSTALL_NAME ${SCOPE_NAME})
  _pkgconfig_invoke(libunity-scopes "SCOPE" "DIR" "" --variable=scopesdir)
  set(SCOPE_INSTALL_DIR "${SCOPE_DIR}/${SCOPE_NAME}")
endif()
set(SYMBOL_MAP "${CMAKE_SOURCE_DIR}/data/${SCOPE_NAME}.map")

add_subdirectory(src)
add_subdirectory(data)

if(NOT CLICK_MODE)
  enable_testing()
  add_subdirectory(tests)
  add_custom_target(
        check
        ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure
  )
endif()
