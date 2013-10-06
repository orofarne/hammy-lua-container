include_directories("${PROJECT_SOURCE_DIR}/lib")

add_executable(hammyd main.cpp)
add_dependencies(hammyd
    hammy_lib
    luajit
    luacmsgpack_lib
)
target_link_libraries(hammyd
    hammy_lib
    luacmsgpack_lib
    hammy_lua_global
    ${Boost_LIBRARIES}
    ${LUAJIT_LIBRARY}
    ${MsgPack_LIBRARY}
    ${MSGPACKCLEN_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    "dl"
)

install(TARGETS hammyd DESTINATION bin)
