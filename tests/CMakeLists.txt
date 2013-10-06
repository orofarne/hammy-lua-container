include_directories("${PROJECT_SOURCE_DIR}/lib")

add_subdirectory("test_plugin")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_config.h.t ${CMAKE_CURRENT_BINARY_DIR}/test_config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB LUTOP_TEST_SOURCES "*.cpp")
add_executable(hammy_test ${LUTOP_TEST_SOURCES})

add_dependencies(hammy_test
  hammy_lib
  luacmsgpack_lib
  gtest
  hammy_test_plugin
)

target_link_libraries(hammy_test
  hammy_lib
  luacmsgpack_lib
  hammy_lua_global
  ${GTEST_BOTH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${LUAJIT_LIBRARY}
  ${MsgPack_LIBRARY}
  ${MSGPACKCLEN_LIBRARY}
  "dl"
)

run_test(hammy_test)
