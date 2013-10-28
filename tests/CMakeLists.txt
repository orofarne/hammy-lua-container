include_directories("${PROJECT_SOURCE_DIR}/src")

file(GLOB LUTOP_TEST_SOURCES "*.cpp")
add_executable(hammy_test ${LUTOP_TEST_SOURCES})

add_dependencies(hammy_test
  hammy_lib
  luacmsgpack_lib
  gtest
)

target_link_libraries(hammy_test
  hammy_lua_container_lib
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
