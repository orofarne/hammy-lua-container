[hammy]
plugin_path = "${CMAKE_BINARY_DIR}/plugins"
bus_plugin = "internal_bus"
state_keeper_plugin = "leveldb_sk"

[internal_bus]

[leveldb_sk]
dbfile = "${CMAKE_CURRENT_BINARY_DIR}/example.db"
