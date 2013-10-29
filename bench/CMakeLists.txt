include_directories("${PROJECT_SOURCE_DIR}/src")

file(GLOB HAMMY_BENCH_SOURCES "*.cpp")
add_executable(hammy_bench ${HAMMY_BENCH_SOURCES})

add_dependencies(hammy_bench
  hammy_lib
  celero
)

target_link_libraries(hammy_bench
  hammy_lua_container_lib
  luacmsgpack_lib
  hammy_lua_global
  ${CELERO_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${LUAJIT_LIBRARY}
  ${MsgPack_LIBRARY}
  ${MSGPACKCLEN_LIBRARY}
  "dl"
)

run_bench(hammy_bench)

