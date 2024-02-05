add_executable(server)
target_sources(server PRIVATE server.cpp avl.cpp hashtable.cpp zset.cpp list.h
                              thread_pool.cpp)

add_executable(client)
target_sources(client PRIVATE client.cpp)

add_executable(test_avl)
target_sources(test_avl PRIVATE test_avl.cpp avl.cpp)
