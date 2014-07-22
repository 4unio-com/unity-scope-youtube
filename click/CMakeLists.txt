
configure_file(
  "manifest.json.in"
  "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
  @ONLY
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
    "apparmor.json"
  DESTINATION "click"
)

