
# !!!!!!!!!!!!! EDIT THE FOLLOWING LINE AS NEEDED TO POINT AT YOUR BASIC GRAPHICS LIBARARY INSTALL !!!!!!!!!!!!! 
list(APPEND CMAKE_PREFIX_PATH ../basicgraphics/build/install ../minvr/build/install )

message(STATUS "prefix path: ${CMAKE_PREFIX_PATH}")

# Using 3.9 to get a modern version of FindOpenGL.cmake
cmake_minimum_required (VERSION 3.9) 

set (CMAKE_CXX_STANDARD 11)

# Dependencies that are auto-downloaded, built, and installed for you will go here.  So, you 
# may use this even if not planning to install this particular project on your system.
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE )
endif()

list(APPEND CMAKE_MODULE_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_INSTALL_PREFIX})
list(INSERT CMAKE_PREFIX_PATH 0 ${CMAKE_INSTALL_PREFIX})

include(AutoBuild)
AutoBuild_init()


#---------------------- Source for this Project ----------------------

project("you-animated")

message(STATUS "==== BUILDING ${PROJECT_NAME}")
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")



set(source_files
  src/main.cpp
  src/App.cpp
  src/Model.cpp
  src/Mesh.cpp
)

set(header_files
  src/App.hpp
  src/Model.h
  src/Mesh.h
)

set(extra_files
  shaders/vertex.glsl
  shaders/fragment.glsl
  README.md
)

set_source_files_properties(${extra_files} PROPERTIES HEADER_FILE_ONLY TRUE)



#---------------------- Define the Target ----------------------


add_executable(${PROJECT_NAME} ${source_files} ${header_files} ${extra_files})



#---------------------- Find & Add Dependencies ----------------------

set(EXTERNAL_DIR_NAME external)
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${EXTERNAL_DIR_NAME})
set(EXTERNAL_CMAKE_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# MinVR (linked with an imported cmake target so no need to specify include dirs)
find_package(BasicGraphics REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC BasicGraphics::BasicGraphics)

find_package(MinVR REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC MinVR::MinVR)

# OpenGL
include(AutoBuildOpenGL)
AutoBuild_use_package_OpenGL(${PROJECT_NAME} PUBLIC)


# GLM
#AutoBuild_use_package(glm . ${PROJECT_NAME} PUBLIC glm
#    DOWNLOAD_ARGS  GIT_REPOSITORY "https://github.com/g-truc/glm.git" GIT_TAG master
#)

if (NOT MSVC)
   find_package(ZLIB REQUIRED)
endif()

if (APPLE)
   find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
   target_link_libraries(${PROJECT_NAME} PUBLIC ${CORE_FOUNDATION_FRAMEWORK})
endif()

# GLEW
if (NOT APPLE)
    include(AutoBuildGLEW)
    AutoBuild_use_package_GLEW(${PROJECT_NAME} PUBLIC)
endif()


if (WIN32)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "./Debug")
	#set_target_properties(${WINDOWS_BINARIES} PROPERTIES VS_STARTUP_PROJECT ${PROJECT_NAME})
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif(WIN32)

#copy resource files to build folder so that the executable can find them in the working directory
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy_directory
#                       ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>
#					   )
			
#copy the basic graphics resources to the build directory			
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${BasicGraphics_DIR}/../../../../../resources $<TARGET_FILE_DIR:${PROJECT_NAME}>
					   )

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
  #copy assimp dll files to build folder so that the executable can find them in the working directory
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy
                         ${BasicGraphics_DIR}/../../../bin/assimp${ASSIMP_LIBRARY_SUFFIX}.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
		  			   )

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy
                         ${MinVR_DIR}/../../../bin/glew32.dll $<TARGET_FILE_DIR:${PROJECT_NAME}> 
		  			   )
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy
                         ${MinVR_DIR}/../../../bin/glew32d.dll $<TARGET_FILE_DIR:${PROJECT_NAME}> 
		  			   )					   
endif()					   
				  

AutoBuild_check_status()
