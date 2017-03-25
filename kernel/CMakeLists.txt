set_property(SOURCE _setup.S x86.S PROPERTY LANGUAGE C)

aux_source_directory(. kernel)
aux_source_directory(mm mm)
aux_source_directory(rootfs rootfs)
aux_source_directory(drivers drivers)
aux_source_directory(drivers/input input)

enable_language(ASM-ATT)
add_executable(none 
    _setup.S
    x86.S
    ${kernel}
    ${mm}
    ${rootfs}
    ${drivers}
    ${input})

target_link_libraries(none
    -Wl,--start-group
    posix
    c
    fs
    -Wl,--whole-archive
    -Wl,--no-whole-archive
    -Wl,--end-group
    )
install(TARGETS 
    none
    DESTINATION boot/)
