
configure_file(
  "manifest.json.in"
  "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
  @ONLY
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
    "apparmor.json"
    "youtube.application"
    "youtube.service"
    "youtube.desktop"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

