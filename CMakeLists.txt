cmake_minimum_required(VERSION 3.16)

project(micran-tree-cache
    VERSION 0.1.0
    DESCRIPTION "Локальный кэш древовидной базы данных"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Тип сборки" FORCE)
endif()


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 5.15 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} 5.15 REQUIRED COMPONENTS Widgets)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
include(ClangTools)

add_library(domain STATIC
    src/domain/TreeNode.cpp
    src/domain/TreeNode.h
    src/domain/NodeId.h
    src/domain/NodeStatus.h
    src/domain/DatabaseTypes.h
    src/domain/IDatabaseRepository.h
)

target_include_directories(domain PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(domain PRIVATE
    project_warnings
)

add_library(infrastructure STATIC
    src/infrastructure/InMemoryDatabase.cpp
    src/infrastructure/InMemoryDatabase.h
    src/infrastructure/DatabaseRepository.cpp
    src/infrastructure/DatabaseRepository.h
)

target_include_directories(infrastructure PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(infrastructure
    PUBLIC domain
    PRIVATE project_warnings
)

add_library(application STATIC
    src/application/CacheService.cpp
    src/application/CacheService.h
)

target_include_directories(application PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(application
    PUBLIC domain
    PRIVATE project_warnings
)

add_library(presentation STATIC
    src/presentation/TreeModel.cpp
    src/presentation/TreeModel.h
    src/presentation/CacheTreeProvider.cpp
    src/presentation/CacheTreeProvider.h
    src/presentation/NodeItemDelegate.cpp
    src/presentation/NodeItemDelegate.h
    src/presentation/ITreeDataProvider.h
)

target_include_directories(presentation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(presentation
    PUBLIC domain application Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE project_warnings
)

set(PROJECT_SOURCES
    src/main.cpp
    src/presentation/mainwindow.cpp
    src/presentation/mainwindow.h
    src/presentation/mainwindow.ui
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(micran-tree-cache ${PROJECT_SOURCES})
else()
    add_executable(micran-tree-cache ${PROJECT_SOURCES})
endif()

target_include_directories(micran-tree-cache PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(micran-tree-cache PRIVATE
    domain
    Qt${QT_VERSION_MAJOR}::Widgets
    project_warnings
)

option(BUILD_TESTING "Собирать юнит-тесты" ON)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()