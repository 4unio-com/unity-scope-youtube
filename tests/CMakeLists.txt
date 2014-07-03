
include(FindGMock)

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS}
)

add_definitions(
  -DFAKE_YOUTUBE_SERVER="${CMAKE_CURRENT_SOURCE_DIR}/server/server.py"
)

add_subdirectory(functional)
add_subdirectory(unit)
