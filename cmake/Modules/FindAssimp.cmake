# - Find Assimp Installation
#
# Users can set the following variables before calling the module:
#  ASSIMP_DIR - The preferred installation prefix for searching for ASSIMP. Set by the user.
#
# ASSIMP_INCLUDE_DIRS - include directories
# ASSIMP_LIBRARIES - libraries to link plugins with

find_path(ASSIMP_INCLUDE_DIR 
    assimp/defs.h
  NO_DEFAULT_PATH
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/include 
    ${CMAKE_INSTALL_PREFIX}/include/assimp
)
if (NOT ASSIMP_INCLUDE_DIR)
    find_path(ASSIMP_INCLUDE_DIR 
        assimp/defs.h
      HINTS 
        ${CMAKE_INSTALL_PREFIX}/include 
        ${CMAKE_INSTALL_PREFIX}/include/assimp
        $ENV{ASSIMP_DIR}/include 
        $ENV{ASSIMP_DIR}/include/assimp 
        /usr/local/include
        /usr/local/include/assimp
    )
endif()


if( MSVC )
  # in order to prevent DLL hell, each of the DLLs have to be suffixed with the major version and msvc prefix
  if( MSVC70 OR MSVC71 )
    set(MSVC_PREFIX "vc70")
  elseif( MSVC80 )
    set(MSVC_PREFIX "vc80")
  elseif( MSVC90 )
    set(MSVC_PREFIX "vc90")
  elseif( MSVC10 )
    set(MSVC_PREFIX "vc100")
  elseif( MSVC11 )
    set(MSVC_PREFIX "vc110")
  elseif( MSVC12 )
    set(MSVC_PREFIX "vc120")
  elseif( MSVC14 )
    set(MSVC_PREFIX "vc140")
  else()
    set(MSVC_PREFIX "vc150")
  endif()
  set(ASSIMP_LIBRARY_SUFFIX "-${MSVC_PREFIX}-mt" CACHE STRING "the suffix for the assimp windows library" )
else()
  set(ASSIMP_LIBRARY_SUFFIX "" CACHE STRING "the suffix for the assimp libraries" )
endif()

find_library(ASSIMP_LIBRARY 
  NAMES 
    libassimp.a 
    assimp${ASSIMP_LIBRARY_SUFFIX}.lib 
    assimp
  NO_DEFAULT_PATH
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/lib 
)
if (NOT ASSIMP_LIBRARY)
    find_library(ASSIMP_LIBRARY 
      NAMES 
        libassimp.a 
    assimp${ASSIMP_LIBRARY_SUFFIX}.lib 
    assimp
      HINTS 
        ${CMAKE_INSTALL_PREFIX}/lib 
        $ENV{ASSIMP_DIR}/lib 
        /usr/local/lib
    )
endif()

find_library(IRRXML_LIBRARY 
  NAMES 
    IrrXML.lib
    libIrrXML.a 
	IrrXML
  NO_DEFAULT_PATH
  HINTS 
    ${CMAKE_INSTALL_PREFIX}/lib 
)
if (NOT IRRXML_LIBRARY)
    find_library(IRRXML_LIBRARY 
      NAMES 
		IrrXML.lib
        libIrrXML.a 
		IrrXML
      HINTS 
        ${CMAKE_INSTALL_PREFIX}/lib 
        $ENV{ASSIMP_DIR}/lib 
        /usr/local/lib
    )
endif()

find_package(ZLIB QUIET)
if (NOT ZLIB_FOUND)
	if (MSVC)
		find_library(ZLIB_LIBRARY 
		  NAMES 
			zlibstaticd.lib
		  NO_DEFAULT_PATH
		  HINTS 
			${CMAKE_INSTALL_PREFIX}/lib 
		)
		if (NOT ZLIB_LIBRARY)
			find_library(ZLIB_LIBRARY 
			  NAMES 
				zlibstaticd.lib
			  HINTS 
				${CMAKE_INSTALL_PREFIX}/lib 
				$ENV{ASSIMP_DIR}/lib 
				/usr/local/lib
			)
		endif()
		set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
	endif()
endif()

set(ASSIMP_LIBRARIES ${ASSIMP_LIBRARY} ${IRRXML_LIBRARY} ${ZLIB_LIBRARIES})
set( ASSIMP_INCLUDE_DIRS "${ASSIMP_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    ASSIMP
    DEFAULT_MSG
    ASSIMP_INCLUDE_DIR
    ASSIMP_LIBRARY 
    IRRXML_LIBRARY 
    ASSIMP_LIBRARIES
	ZLIB_LIBRARY
)

mark_as_advanced(
    ASSIMP_INCLUDE_DIR
	ASSIMP_LIBRARY 
    IRRXML_LIBRARY 
	ZLIB_LIBRARY
    ASSIMP_LIBRARIES	
)
