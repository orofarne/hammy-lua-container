add_library(hammy_lib STATIC
    Value.cpp
    Manager.cpp
    Context.cpp
    ProcessPool.cpp
    SubProcess.cpp
    Worker.cpp
    Module.cpp
    luatools.cpp
    Encode.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lua_cmsgpack.c ${CMAKE_CURRENT_BINARY_DIR}/lua_cmsgpack.c)
add_library(luacmsgpack_lib STATIC ${CMAKE_CURRENT_BINARY_DIR}/lua_cmsgpack.c)
add_dependencies(luacmsgpack_lib luacmsgpack)

add_executable(hammyd hammyd.cpp)
add_dependencies(hammyd
    hammy_lib
    luajit
    luacmsgpack_lib
)
target_link_libraries(hammyd
    hammy_lib
    luacmsgpack_lib
    ${Boost_LIBRARIES}
    ${LUAJIT_LIBRARY}
    ${MsgPack_LIBRARY}
    ${MSGPACKCLEN_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    "dl"
)

install(TARGETS hammyd DESTINATION bin)
