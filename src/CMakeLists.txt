if(NOT CMAKE_CONFIGURATION_TYPES MATCHES Debug)
    find_program(CLANG_TIDY clang-tidy)
    message(STATUS "Clang-tidy: ${CLANG_TIDY}")
    if(CLANG_TIDY)
        file(STRINGS ${PROJECT_SOURCE_DIR}/clang-tidy-checks.txt clang_tidy_checks)
        list(JOIN clang_tidy_checks "," clang_tidy_checks_str)
        set(CMAKE_C_CLANG_TIDY
                ${CLANG_TIDY}
                -header-filter=${CMAKE_SOURCE_DIR}
                -checks=${clang_tidy_checks_str}
        )
    endif()
endif()

set(sources
        backend/io/file.c
        utils/logger.c
        backend/io/caching.c
        backend/io/pager.c
        backend/pstack/pstack.c
        backend/page_pool/page_pool.c
        backend/io/linked_pages.c
        backend/utils/parray.c
        backend/page_pool/linked_blocks.c
        backend/utils/parray64.c
        backend/table/schema.c
        backend/table/table_base.c
        backend/table/table.c
        backend/journal/metatab.c
        backend/journal/materializer.c
        backend/journal/varchar_mgr.c
        backend/comporator/comporator.c
        backend/db/db.c

)
#file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.c *.h)
#list(FILTER sources EXCLUDE REGEX main.c)

add_library(db STATIC ${sources})
target_include_directories(db PUBLIC .)
target_link_libraries(db PUBLIC m)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
    add_executable(db_exe main.c)
    target_link_libraries(db_exe PRIVATE db)
    set_target_properties(db_exe PROPERTIES OUTPUT_NAME db)
endif()

