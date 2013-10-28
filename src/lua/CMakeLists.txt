set(lua_global_SRC "${CMAKE_CURRENT_BINARY_DIR}/lua_global.cpp")
file(GLOB lua_global_in_SRC "*.lua")

add_custom_command(OUTPUT "${lua_global_SRC}"
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen.sh ${lua_global_in_SRC} > lua_global.cpp
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gen.sh ${lua_global_in_SRC}
)

add_library(hammy_lua_global STATIC "${lua_global_SRC}")
