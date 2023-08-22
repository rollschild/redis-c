include(GoogleTest)

add_executable(foo_test)
target_sources(foo_test PRIVATE foo_test.c)
target_link_libraries(foo_test PRIVATE gtest gmock gtest_main)
gtest_discover_tests(foo_test)
