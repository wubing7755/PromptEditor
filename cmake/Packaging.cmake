include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(PP_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/PromptLib")

install(TARGETS promptlib_core
    EXPORT PromptLibTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${PP_GENERATED_INCLUDE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PromptLibConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PromptLibConfig.cmake"
    INSTALL_DESTINATION "${PP_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/PromptLibConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(EXPORT PromptLibTargets
    NAMESPACE PromptLib::
    DESTINATION ${PP_INSTALL_CMAKEDIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/PromptLibConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/PromptLibConfigVersion.cmake"
    DESTINATION ${PP_INSTALL_CMAKEDIR}
)
