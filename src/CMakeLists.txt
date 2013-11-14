add_library(hammy_lua_container_lib STATIC
	reader.c
	writer.c
	child.c
	router.c
	worker.c
	lua_eval.c
	${LUACMSGPACK_SRC}
)

add_executable(hammy_lua_container main.c)
add_dependencies(hammy_lua_container hammy_lua_container_lib)
target_link_libraries(hammy_lua_container
	hammy_lua_container_lib
	${GLIB2_LIBRARIES}
	${EV_LIBRARY}
	${MSGPACKCLEN_LIBRARY}
)
