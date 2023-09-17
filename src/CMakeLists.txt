add_executable(server)
target_sources(server PRIVATE server.cpp)

add_executable(client)
target_sources(client PRIVATE client.cpp)

add_executable(test_avl)
target_sources(test_avl PRIVATE test_avl.cpp avl.cpp)
