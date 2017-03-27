add_executable(ls main.c)

target_link_libraries(ls
    -Wl,--start-group
    c
    posix
    -Wl,--end-group
    )

install(TARGETS
    ls
    DESTINATION ramdisk/bin/
    )
