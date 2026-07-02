find_package(Qt${QT_VERSION_MAJOR} 5.15 REQUIRED COMPONENTS Test)

add_executable(test_smoke test_smoke.cpp)
add_executable(test_tree_node test_tree_node.cpp)

target_link_libraries(test_smoke PRIVATE Qt${QT_VERSION_MAJOR}::Test project_warnings)
target_link_libraries(test_tree_node PRIVATE Qt${QT_VERSION_MAJOR}::Test domain project_warnings)

add_test(NAME smoke COMMAND test_smoke)
add_test(NAME tree_node COMMAND test_tree_node)
