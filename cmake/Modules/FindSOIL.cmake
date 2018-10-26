# - Find SOIL Installation
#
# Users can set the following variables before calling the module:
#  SOIL_DIR - The preferred installation prefix for searching for SOIL. Set by the user.
#
# SOIL_INCLUDE_DIRS - include directories
# SOIL_LIBRARIES - libraries to link plugins with

find_path(SOIL_INCLUDE_DIR 
    SOIL.h
  NO_DEFAULT_PATH
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/include 
)
if (NOT SOIL_INCLUDE_DIR)
    find_path(SOIL_INCLUDE_DIR 
        SOIL.h
      HINTS 
        ${CMAKE_INSTALL_PREFIX}/include 
        $ENV{SOIL_DIR}/include 
        /usr/local/include
    )
endif()

find_library(SOIL_LIBRARY 
  NAMES 
    libSOIL.a 
    SOIL.lib 
    SOIL
  NO_DEFAULT_PATH
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/lib 
)
if (NOT SOIL_LIBRARY)
    find_library(SOIL_LIBRARY 
      NAMES 
        libSOIL.a 
		SOIL.lib 
		SOIL
      HINTS 
        ${CMAKE_INSTALL_PREFIX}/lib 
        $ENV{SOIL_DIR}/lib 
        /usr/local/lib
    )
endif()

set(SOIL_LIBRARIES ${SOIL_LIBRARY})
set( SOIL_INCLUDE_DIRS "${SOIL_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    SOIL
    DEFAULT_MSG
    SOIL_INCLUDE_DIR
    SOIL_LIBRARY 
    SOIL_LIBRARIES
)

mark_as_advanced(
    SOIL_INCLUDE_DIR
	SOIL_LIBRARY
    SOIL_LIBRARIES	
)
