cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#if (NOT DEFINED EMSCRIPTEN)
#  message(FATAL_ERROR "SkiaWebBindings need to be compiled with emscripten to WASM")
#endif()

if (NOT DEFINED SKIA_ROOT_DIR)
  message(FATAL_ERROR "Need set SKIA_ROOT_DIR to point to the directory with the skia checkout.")
endif()


if (NOT DEFINED SKIA_LIB_DIR)
  message(FATAL_ERROR "Need set SKIA_LIB_DIR to point to the directory containing libSkia.a compiled to WASM.")
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_FIND_PACKAGE_REDIRECTS_DIR ${CMAKE_BINARY_DIR})

# set the project name and version
project(SkiaWebBinding VERSION 0.1)

message("CMAKE TARGET SYSTEM IS " ${CMAKE_SYSTEM_NAME})

message(${PROJECT_NAME} ": compiler id is " ${CMAKE_CXX_COMPILER_ID} " VERSION IS " ${CMAKE_CXX_COMPILER_VERSION})

# add compile caching if ccache is installed on the system
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CXX_COMPILER_LAUNCHER  "${CCACHE_PROGRAM}" CACHE INTERNAL "")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  message("Enabled compiler caching")
endif()

add_compile_options(-fwasm-exceptions)
add_compile_options(-pthread)

add_library(SkiaWebBinding STATIC
  "src/module.cpp"
  "src/skiaLogo.cpp"
  "src/skiaWebBinding.cpp")

target_include_directories(SkiaWebBinding
  PUBLIC
  include
  ${SKIA_ROOT_DIR}
)
