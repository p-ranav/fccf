include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package fccf)

install(
    TARGETS fccf_exe
    RUNTIME COMPONENT fccf_Runtime
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    fccf_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(fccf_INSTALL_CMAKEDIR)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${fccf_INSTALL_CMAKEDIR}"
    COMPONENT fccf_Development
)

# Export variables for the install script to use
install(CODE "
set(fccf_NAME [[$<TARGET_FILE_NAME:fccf_exe>]])
set(fccf_INSTALL_CMAKEDIR [[${fccf_INSTALL_CMAKEDIR}]])
set(CMAKE_INSTALL_BINDIR [[${CMAKE_INSTALL_BINDIR}]])
" COMPONENT fccf_Development)

install(
    SCRIPT cmake/install-script.cmake
    COMPONENT fccf_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
