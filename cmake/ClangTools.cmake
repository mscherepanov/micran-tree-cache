file(GLOB_RECURSE MICRAN_ALL_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
)

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)

if(CLANG_FORMAT_EXECUTABLE)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i --style=file ${MICRAN_ALL_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Форматирование исходного кода (clang-format)"
        VERBATIM
    )

    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror --style=file
                ${MICRAN_ALL_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Проверка форматирования (clang-format --dry-run)"
        VERBATIM
    )

    message(STATUS "clang-format найден: доступны цели 'format' и 'format-check'")
else()
    message(STATUS "clang-format не найден: цели форматирования отключены")
endif()

find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
find_program(RUN_CLANG_TIDY_EXECUTABLE NAMES run-clang-tidy run-clang-tidy.py)

if(CLANG_TIDY_EXECUTABLE)
    if(RUN_CLANG_TIDY_EXECUTABLE)
        add_custom_target(tidy
            COMMAND ${RUN_CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR}
                    -quiet
                    ${CMAKE_SOURCE_DIR}/src
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Статический анализ (run-clang-tidy по compile_commands.json)"
            VERBATIM
        )
        message(STATUS "clang-tidy найден: доступна цель 'tidy' (run-clang-tidy)")
    else()
        file(GLOB_RECURSE MICRAN_TIDY_SOURCES CONFIGURE_DEPENDS
            ${CMAKE_SOURCE_DIR}/src/*.cpp)
        add_custom_target(tidy
            COMMAND ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR}
                    --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
                    ${MICRAN_TIDY_SOURCES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Статический анализ (clang-tidy)"
            VERBATIM
        )
        message(STATUS "clang-tidy найден: доступна цель 'tidy' "
                       "(run-clang-tidy не найден, используется clang-tidy)")
    endif()

    option(ENABLE_CLANG_TIDY "Запускать clang-tidy при сборке" OFF)
    if(ENABLE_CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY_EXECUTABLE} --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy)
        message(STATUS "clang-tidy включён в процесс сборки (ENABLE_CLANG_TIDY=ON)")
    endif()
else()
    message(STATUS "clang-tidy не найден: цель анализа отключена")
endif()