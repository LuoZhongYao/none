aux_source_directory(. src)
add_executable(v6sh ${src})

target_link_libraries(v6sh
    -Wl,--start-group
    posix
    c
    -Wl,--whole-archive
    -Wl,--no-whole-archive
    -Wl,--end-group
    )

install(TARGETS 
    v6sh
    DESTINATION ramdisk/bin/
    )
