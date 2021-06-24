cmake_minimum_required(VERSION 3.18)
project(libsettings)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# nlohmann_json
find_package(nlohmann_json REQUIRED)
get_target_property(
  NLOHMANN_JSON_INC
  nlohmann_json::nlohmann_json
  INTERFACE_INCLUDE_DIRECTORIES
  )

# zlib
find_library(ZLIB_A NAMES libz.a PATHS ${CMAKE_INSTALL_RPATH})
if(ZLIB_A MATCHES "NOTFOUND")
  message(FATAL_ERROR "libz.a not found")
endif()
string(REPLACE lib/libz.a include/ ZLIB_INC ${ZLIB_A})
if(NOT EXISTS ${ZLIB_INC})
  message(FATAL_ERROR "zlib include dir doesn't exist")
endif()

# libprim
find_library(LIBPRIM_A NAMES libprim.a PATHS ${CMAKE_INSTALL_RPATH})
if(LIBPRIM_A MATCHES "NOTFOUND")
  message(FATAL_ERROR "libprim.a not found")
endif()
string(REPLACE lib/libprim.a include/ LIBPRIM_INC ${LIBPRIM_A})
if(NOT EXISTS ${LIBPRIM_INC})
  message(FATAL_ERROR "libprim include dir doesn't exist")
endif()

# libstrop
find_library(LIBSTROP_A NAMES libstrop.a PATHS ${CMAKE_INSTALL_RPATH})
if(LIBSTROP_A MATCHES "NOTFOUND")
  message(FATAL_ERROR "libstrop.a not found")
endif()
string(REPLACE lib/libstrop.a include/ LIBSTROP_INC ${LIBSTROP_A})
if(NOT EXISTS ${LIBSTROP_INC})
  message(FATAL_ERROR "libstrop include dir doesn't exist")
endif()

# libfio
find_library(LIBFIO_A NAMES libfio.a PATHS ${CMAKE_INSTALL_RPATH})
if(LIBFIO_A MATCHES "NOTFOUND")
  message(FATAL_ERROR "libfio.a not found")
endif()
string(REPLACE lib/libfio.a include/ LIBFIO_INC ${LIBFIO_A})
if(NOT EXISTS ${LIBFIO_INC})
  message(FATAL_ERROR "libfio include dir doesn't exist")
endif()

add_library(
  settings
  STATIC
  ${PROJECT_SOURCE_DIR}/src/settings/settings.cc
  ${PROJECT_SOURCE_DIR}/src/settings/settings.h
  )

set_target_properties(
  settings
  PROPERTIES
  LINKER_LANGUAGE
  CXX
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
  ${ZLIB_A}
  ${LIBPRIM_A}
  ${LIBSTROP_A}
  ${LIBFIO_A}
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

