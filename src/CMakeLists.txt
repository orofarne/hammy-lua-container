add_subdirectory(lua)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lua_cmsgpack.c ${CMAKE_CURRENT_BINARY_DIR}/lua_cmsgpack.c)
add_library(luacmsgpack_lib STATIC ${CMAKE_CURRENT_BINARY_DIR}/lua_cmsgpack.c)
add_dependencies(luacmsgpack_lib luacmsgpack hammy_lua_global)

add_library(hammy_lua_container_lib STATIC
    Server.cpp
    Client.cpp
    Context.cpp
    ProcessPool.cpp
    SubProcess.cpp
    Worker.cpp
    luatools.cpp
    Router.cpp
)

add_dependencies(hammy_lua_container_lib
    luacmsgpack_lib
)

add_executable(hammy_lua_container main.cpp)
add_dependencies(hammy_lua_container hammy_lua_container_lib)
target_link_libraries(hammy_lua_container
    hammy_lua_container_lib
    luacmsgpack_lib
    hammy_lua_global
    ${Boost_LIBRARIES}
    ${LUAJIT_LIBRARY}
    ${MsgPack_LIBRARY}
    ${MSGPACKCLEN_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    "dl"
)

install(TARGETS hammy_lua_container DESTINATION bin)
