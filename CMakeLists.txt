cmake_minimum_required(VERSION 3.27.7)

project(Simulator)

# set the default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES "source/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# adjusting compile options based on the compiler
if(MSVC)
  # use MSVC-specific warning levels and flags
  target_compile_options(${PROJECT_NAME} PRIVATE "/W4" "/wd4100" "/wd4250")
  set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /D_DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Zi /O2 /D_DEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "/Os")
else()
  # assume GCC or Clang is being used
  target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Wextra")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
endif()
