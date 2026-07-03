include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(PROMPTEDITOR_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/PromptEditor")

install(TARGETS prompteditor_core
    EXPORT PromptEditorTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${PROMPTEDITOR_GENERATED_INCLUDE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PromptEditorConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PromptEditorConfig.cmake"
    INSTALL_DESTINATION "${PROMPTEDITOR_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/PromptEditorConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(EXPORT PromptEditorTargets
    NAMESPACE PromptEditor::
    DESTINATION ${PROMPTEDITOR_INSTALL_CMAKEDIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/PromptEditorConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/PromptEditorConfigVersion.cmake"
    DESTINATION ${PROMPTEDITOR_INSTALL_CMAKEDIR}
)
