include_directories("${PROJECT_SOURCE_DIR}/src")

find_package(CUnit REQUIRED)
include_directories(${CUNIT_INCLUDE_DIR})

set(TEST_SOURCES
	main.c
	router.c
)

add_executable(hammy_lua_container_tests ${TEST_SOURCES})
add_dependencies(hammy_lua_container_tests hammy_lua_container_lib)
target_link_libraries(hammy_lua_container_tests
	${CUNIT_LIBRARY}
	hammy_lua_container_lib
	${GLIB2_LIBRARIES}
	${EV_LIBRARY}
	${MSGPACKCLEN_LIBRARY}
)

set(test_target hammy_lua_container_tests)
add_custom_target(test)
add_custom_target(${test_target}_runtest
	COMMAND ${test_target}
	DEPENDS ${test_target}
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
add_dependencies(test ${test_target}_runtest)
