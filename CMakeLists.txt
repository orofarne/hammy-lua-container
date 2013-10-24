cmake_minimum_required(VERSION 2.8)

project(hammy C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

OPTION(TESTS "enable_testing" ON)
OPTION(PLUGINS "enable_std_plugins" ON)

# Initialize FLAGS.
set(CMAKE_CXX_FLAGS                "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
set(CMAKE_C_FLAGS_DEBUG            "-O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL       "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE          "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")

# Compiler-specific C++11 activation.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
    message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
  endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else ()
  message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

# Setup boost
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package( Boost 1.48.0 COMPONENTS system filesystem program_options thread REQUIRED )
include_directories(${Boost_INCLUDE_DIRS})

# Threads
find_package(Threads REQUIRED)

# Lua staff
add_subdirectory(contrib)
include_directories(${LUAJIT_INCLUDE_DIRS})
include_directories(${MSGPACKCLEN_INCLUDE_DIRS})

# Msgpack for manager process
find_package(MsgPack REQUIRED)
include_directories(${MsgPack_INCLUDE_DIRS})

# Code
add_subdirectory(lib)
add_subdirectory(hammyd)

# Std plugins
if(PLUGINS)
    find_package(LevelDB REQUIRED)
    add_subdirectory(plugins)
    add_subdirectory(example)
endif(PLUGINS)

# GTest magic
if(TESTS)
  message(STATUS "GTest include dir: ${GTEST_INCLUDE_DIR}")
  include_directories(${GTEST_INCLUDE_DIR})

  add_custom_target(test)
  macro(run_test test_target)
    add_custom_target(${test_target}_runtest
        COMMAND ${test_target}
        DEPENDS ${test_target}
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    add_dependencies(test ${test_target}_runtest)
  endmacro()

  add_subdirectory(tests)
endif(TESTS)
