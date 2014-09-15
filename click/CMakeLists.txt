
function(configure_scope_files)
  foreach(_file ${ARGV})
    configure_file(
      "${_file}.in"
      "${CMAKE_CURRENT_BINARY_DIR}/${_file}"
      @ONLY
    )
  endforeach()
endfunction()

configure_scope_files(
  manifest.json
  youtube.service
  youtube.application
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
    "apparmor.json"
    "${CMAKE_CURRENT_BINARY_DIR}/youtube.service"
    "${CMAKE_CURRENT_BINARY_DIR}/youtube.application"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

