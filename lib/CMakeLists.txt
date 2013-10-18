add_subdirectory(lua)

add_library(hammy_lib STATIC
    Application.cpp
    Error.cpp
    Value.cpp
    Manager.cpp
    Context.cpp
    ProcessPool.cpp
    SubProcess.cpp
    Worker.cpp
    Module.cpp
    luatools.cpp
    Encode.cpp
    PluginLoader.cpp
    Timer.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lua_cmsgpack.c ${CMAKE_CURRENT_BINARY_DIR}/lua_cmsgpack.c)
add_library(luacmsgpack_lib STATIC ${CMAKE_CURRENT_BINARY_DIR}/lua_cmsgpack.c)
add_dependencies(luacmsgpack_lib luacmsgpack hammy_lua_global)
