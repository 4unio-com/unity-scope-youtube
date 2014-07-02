
# Put the ini file in the build directory next to the scope
# .so file so test tools can find both easily.
configure_file(
  "${CMAKE_SOURCE_DIR}/data/${SCOPE_NAME}.ini.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${SCOPE_NAME}.ini"
  @ONLY
)

add_library(
  ${SCOPE_NAME}-static OBJECT
  youtube/api/channel.cpp
  youtube/api/channel-section.cpp
  youtube/api/client.cpp
  youtube/api/guide-category.cpp
  youtube/api/playlist.cpp
  youtube/api/playlist-item.cpp
  youtube/api/simple-oauth.cpp
  youtube/api/search-list-response.cpp
  youtube/api/video.cpp
  youtube/api/video-category.cpp
  youtube/scope/preview.cpp
  youtube/scope/query.cpp
  youtube/scope/scope.cpp
)

set_target_properties(
  ${SCOPE_NAME}-static
  PROPERTIES
    LINK_FLAGS "-Wl,--export-all-symbols"
)

add_library(
  ${SCOPE_NAME} SHARED
  $<TARGET_OBJECTS:${SCOPE_NAME}-static>
)

target_link_libraries(
  ${SCOPE_NAME}
  ${SCOPE_LDFLAGS}
  ${Boost_LIBRARIES}
)

set_target_properties(
  ${SCOPE_NAME}
  PROPERTIES
    LINK_FLAGS "-Wl,--no-undefined -Wl,--version-script,${SYMBOL_MAP}"
    LINK_DEPENDS "${SYMBOL_MAP}"
)

install(
  TARGETS ${SCOPE_NAME}
  LIBRARY DESTINATION ${SCOPE_INSTALL_DIR}
)
