[hammy]
plugin_path = ${CMAKE_BINARY_DIR}/plugins
bus_plugin = internal_bus
state_keeper_plugin = leveldb_sk
pool_size = 2
worker_lifetime = 100
trigger_path = ${CMAKE_SOURCE_DIR}/example/triggers

[internal_bus]
foo = bar

[leveldb_sk]
dbfile = ${CMAKE_CURRENT_BINARY_DIR}/example.db
