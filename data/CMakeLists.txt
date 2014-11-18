
intltool_merge_translations(
  "${CMAKE_CURRENT_SOURCE_DIR}/${SCOPE_NAME}.ini.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${SCOPE_INSTALL_NAME}.ini"
  ALL
  UTF8
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${SCOPE_INSTALL_NAME}.ini"
  DESTINATION ${SCOPE_INSTALL_DIR}
)

install(
  FILES
    "icon.png"
    "logo.png"
    "screenshot.jpg"
  DESTINATION
    "${SCOPE_INSTALL_DIR}"
)
