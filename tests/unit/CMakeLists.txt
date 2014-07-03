add_executable(
  ${SCOPE_NAME}-unit-tests
  youtube/scope/test-youtube-scope.cpp
  $<TARGET_OBJECTS:${SCOPE_NAME}-static>
)

target_link_libraries(
  ${SCOPE_NAME}-unit-tests
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
  ${SCOPE_LDFLAGS}
  ${Boost_LIBRARIES}
)

add_test(
  ${SCOPE_NAME}-unit-tests
  ${SCOPE_NAME}-unit-tests
)
