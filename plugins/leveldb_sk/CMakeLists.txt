include_directories(${LEVELDB_INCLUDE})

add_library(leveldb_sk SHARED LevelDBStateKeeper.cpp)
target_link_libraries(leveldb_sk ${LEVELDB_LIBS})
