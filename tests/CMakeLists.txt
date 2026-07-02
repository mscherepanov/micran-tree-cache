find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

add_executable(test_smoke test_smoke.cpp)

target_link_libraries(test_smoke PRIVATE Qt${QT_VERSION_MAJOR}::Test project_warnings)

add_test(NAME smoke COMMAND test_smoke)
