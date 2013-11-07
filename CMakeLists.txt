cmake_minimum_required(VERSION 2.8)

project(hammy-lua-container C)

OPTION(TESTS "enable_testing" ON)

# "Vendor" staff
add_subdirectory(vendor)
include_directories(${LUAJIT_INCLUDE_DIRS})
include_directories(${MSGPACKCLEN_INCLUDE_DIRS})

# Code
add_subdirectory(src)

# Tests
if(TESTS)
  add_subdirectory(tests)
endif(TESTS)

