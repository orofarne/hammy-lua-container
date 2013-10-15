include_directories("../lib")

add_library(internal_bus SHARED InternalBus.cpp)
include_directories(${LEVELDB_INCLUDE})

add_library(leveldb_sk SHARED LevelDBStateKeeper.cpp)
target_link_libraries(leveldb_sk ${LEVELDB_LIBS})

add_library(file_cl SHARED FileCodeLoader.cpp)
# target_link_libraries(file_cl ${Boost_LIBRARIES})
