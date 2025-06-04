cmake_minimum_required (VERSION 3.21 FATAL_ERROR)

project (breakout)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

#! ! ! ! ! ! !
#set this to true to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)
#delete the out folder after changing if visual studio doesn recognize the change!
 
# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(thirdparty/SDL) 
add_subdirectory(thirdparty/glm)

##############################################################################

# Automatically add all .c and .h
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS 
   "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" 
   "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable (${PROJECT_NAME} ${ENGINE_SOURCES}) 

if(PRODUCTION_BUILD)
   # setup the ASSETS_PATH macro to be in the root folder of your exe
   target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") 

   # remove the option to debug asserts.
   target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1) 
   target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPLEMT_BUILD=0) 

else()
   # This is useful to get an ASSETS_PATH in your IDE during development
   target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
   target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0) 
   target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPLEMT_BUILD=1) 

endif()

target_link_libraries(${PROJECT_NAME} PRIVATE 
   SDL3::SDL3
   glm::glm-header-only
)

target_include_directories(${PROJECT_NAME} PRIVATE
   "${CMAKE_CURRENT_SOURCE_DIR}/src/"
)