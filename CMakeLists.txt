project(unity-scope-youtube C CXX)
cmake_minimum_required(VERSION 2.8.10)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

add_definitions(
  -fno-permissive
  -std=c++11
  -pedantic
  -Wall
  -Wextra
  -fPIC
)

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)
find_package(Intltool REQUIRED)
find_package(Threads REQUIRED)

find_package(
  Boost
  COMPONENTS
    iostreams
  REQUIRED
)

pkg_check_modules(
  SCOPE
  libunity-scopes>=0.6.7
  jsoncpp
  net-cpp>=1.1.0
  process-cpp
  REQUIRED
)

include_directories(
  "${CMAKE_SOURCE_DIR}/include"
  ${Boost_INCLUDE_DIRS}
  ${SCOPE_INCLUDE_DIRS}
)

set(SCOPE_VERSION "1.5.1-${BZR_REVNO}")
set(CMAKE_INSTALL_PREFIX /)
set(SCOPE_NAME "com.ubuntu.scopes.youtube")
set(SCOPE_INSTALL_NAME "${SCOPE_NAME}_youtube")
set(SYMBOL_MAP "${CMAKE_SOURCE_DIR}/data/${SCOPE_NAME}.map")
set(GETTEXT_PACKAGE "unity-scope-youtube")

add_definitions(-DGETTEXT_PACKAGE="${GETTEXT_PACKAGE}")
add_definitions(-DSCOPE_NAME="${SCOPE_NAME}")
add_definitions(-DSCOPE_INSTALL_NAME="${SCOPE_INSTALL_NAME}")

# This command figures out the target architecture and puts it into the manifest file
execute_process(
  COMMAND dpkg-architecture -qDEB_HOST_ARCH
  OUTPUT_VARIABLE CLICK_ARCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(SCOPE_INSTALL_DIR "youtube")
add_subdirectory(click)

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)

if(NOT CLICK_MODE)
  enable_testing()
  add_subdirectory(tests)
  add_custom_target(
        check
        ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure
  )
endif()
