cmake_minimum_required(VERSION 2.8)

project(hammy-lua-container C)

OPTION(TESTS "enable_testing" ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(GLIB2)
find_package(EV)

include_directories(
	${GLIB2_INCLUDE_DIRS}
	${EV_INCLUDE_DIR}
)

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

