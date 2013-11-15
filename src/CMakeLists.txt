configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lua_cmsgpack.c ${CMAKE_CURRENT_BINARY_DIR}/lua_cmsgpack.c)
add_library(luacmsgpack_lib STATIC ${CMAKE_CURRENT_BINARY_DIR}/lua_cmsgpack.c)

add_library(hammy_lua_container_lib STATIC
	reader.c
	writer.c
	child.c
	router.c
	worker.c
	lua_eval.c
)

add_dependencies(hammy_lua_container_lib
	luacmsgpack_lib
)

add_executable(hammy_lua_container main.c)
add_dependencies(hammy_lua_container hammy_lua_container_lib)
target_link_libraries(hammy_lua_container
	hammy_lua_container_lib
	luacmsgpack_lib
	${GLIB2_LIBRARIES}
	${EV_LIBRARY}
	${MSGPACKCLEN_LIBRARY}
	${LUAJIT_LIBRARY}
	dl
)
