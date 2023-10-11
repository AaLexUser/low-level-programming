
find_program(CLANG_TIDY clang-tidy)
message(STATUS "Clang-tidy: ${CLANG_TIDY}")
if(CLANG_TIDY)
    file(STRINGS ${PROJECT_SOURCE_DIR}/clang-tidy-checks.txt clang_tidy_checks)
    list(JOIN clang_tidy_checks "," clang_tidy_checks_str)
    set(CMAKE_C_CLANG_TIDY
            ${CLANG_TIDY}
            -header-filter=${CMAKE_SOURCE_DIR}
            -checks=${clang_tidy_checks_str}
#            -warnings-as-errors=*
    )
endif()

#add_executable(${PROJECT_NAME}
#        ../src/main.c
#        ../src/backend/table/table.c
#        ../src/backend/table/schema.c
#        ../src/utils/pool.c
#        ../src/utils/linked_list.c
#        ../src/backend/table/table_manager.c
#        ../src/backend/io/file_manager.c
#        ../src/backend/db/db.c
#)
set(sources
        backend/table/table.c
        backend/table/schema.c
        utils/pool.c
        utils/linked_list.c
        backend/table/table_manager.c
        backend/io/file_manager.c
        backend/db/db.c
        utils/cstring.c
)
#file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.c *.h)
#list(FILTER sources EXCLUDE REGEX main.c)

add_library(db STATIC ${sources})
target_include_directories(db PUBLIC .)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
    add_executable(db_exe main.c)
    target_link_libraries(db_exe PRIVATE db)
    set_target_properties(db_exe PROPERTIES OUTPUT_NAME db)
endif()

