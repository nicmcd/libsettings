cmake_minimum_required(VERSION 3.20)
project(libsettings)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FindPkgConfig)

# zlib
pkg_check_modules(zlib REQUIRED IMPORTED_TARGET zlib)
  get_target_property(
  ZLIB_INC
  PkgConfig::zlib
  INTERFACE_INCLUDE_DIRECTORIES
)

# nlohmann_json
pkg_check_modules(nlohmann_json REQUIRED IMPORTED_TARGET nlohmann_json)
  get_target_property(
  NLOHMANN_JSON_INC
  PkgConfig::nlohmann_json
  INTERFACE_INCLUDE_DIRECTORIES
)

# libprim
pkg_check_modules(libprim REQUIRED IMPORTED_TARGET libprim)
  get_target_property(
  LIBPRIM_INC
  PkgConfig::libprim
  INTERFACE_INCLUDE_DIRECTORIES
)

# libstrop
pkg_check_modules(libstrop REQUIRED IMPORTED_TARGET libstrop)
  get_target_property(
  LIBSTROP_INC
  PkgConfig::libstrop
  INTERFACE_INCLUDE_DIRECTORIES
)

# libfio
pkg_check_modules(libfio REQUIRED IMPORTED_TARGET libfio)
  get_target_property(
  LIBFIO_INC
  PkgConfig::libfio
  INTERFACE_INCLUDE_DIRECTORIES
)

add_library(
  settings
  SHARED
  ${PROJECT_SOURCE_DIR}/src/settings/settings.cc
  ${PROJECT_SOURCE_DIR}/src/settings/settings.h
  )

set_target_properties(
  settings
  PROPERTIES
  LINKER_LANGUAGE
  CXX
  POSITION_INDEPENDENT_CODE
  ON
  )

target_include_directories(
  settings
  PUBLIC
  ${PROJECT_SOURCE_DIR}/src
  ${ZLIB_INC}
  ${NLOHMANN_JSON_INC}
  ${LIBPRIM_INC}
  ${LIBSTROP_INC}
  ${LIBFIO_INC}
  )

target_link_libraries(
  settings
  PkgConfig::zlib
  PkgConfig::nlohmann_json
  PkgConfig::libprim
  PkgConfig::libstrop
  PkgConfig::libfio
  )

include(GNUInstallDirs)

install(
  FILES
  ${PROJECT_SOURCE_DIR}/src/settings/settings.h
  DESTINATION
  ${CMAKE_INSTALL_INCLUDEDIR}/settings/
  )

install(
  TARGETS
  settings
  )

configure_file(
  ${PROJECT_SOURCE_DIR}/libsettings.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/libsettings.pc
  @ONLY
  )
install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/libsettings.pc
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/pkgconfig/
  )
