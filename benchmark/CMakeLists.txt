file(GLOB_RECURSE bench_sources CONFIGURE_DEPENDS
        benchs/page_pool_mem.c
        benchs/linked_blocks_mem.c
        benchs/linked_blocks_wide_mem.c
        benchs/linked_blocks_insert.c
        benchs/linked_blocks_wide_insert.c
)

foreach(bench_source IN LISTS bench_sources)
    string(REPLACE "/" ";" name_components ${bench_source})
    list(GET name_components -1 name)
    string(REGEX REPLACE "\.c$" "" name ${name})
    list(APPEND bench_targets bench_${name})

    add_executable(bench_${name} ${bench_source})
    target_link_libraries(bench_${name} PRIVATE db)
    add_test(NAME bench_${name} COMMAND bench_${name})
endforeach()

set(CMAKE_Cbench_ARGUMENTS --output-junit ${PROJECT_SOURCE_DIR}/report.xml --output-on-failure -C $<CONFIG>)
add_custom_target(bench
        COMMAND ${CMAKE_Cbench_COMMAND} ${CMAKE_Cbench_ARGUMENTS}
        DEPENDS ${bench_targets})
