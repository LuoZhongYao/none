set_property(SOURCE _setup.S x86.S PROPERTY LANGUAGE C)

aux_source_directory(. kernel)
aux_source_directory(mm mm)
aux_source_directory(rootfs rootfs)
aux_source_directory(drivers drivers)
aux_source_directory(drivers/input input)

enable_language(ASM-ATT)


set(ASM-OFFSETS asm-offsets)
set(ASM_OFFSET_PROGM "${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -S -I${CMAKE_CURRENT_SOURCE_DIR} -I${CMAKE_SOURCE_DIR}/include")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_CURRENT_BINARY_DIR}/kernel.lds")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Ttext 0x301000")
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ASM-OFFSETS}.h
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/asm-offsets.def ${CMAKE_CURRENT_BINARY_DIR}/${ASM-OFFSETS}.c
    COMMAND sh -c "${ASM_OFFSET_PROGM} ${CMAKE_CURRENT_BINARY_DIR}/${ASM-OFFSETS}.c -o ${CMAKE_CURRENT_BINARY_DIR}/${ASM-OFFSETS}.s"
    COMMAND ${CMAKE_SOURCE_DIR}/tools/asm-offsets.sh ${CMAKE_CURRENT_BINARY_DIR}/${ASM-OFFSETS}.s ${CMAKE_CURRENT_BINARY_DIR}/${ASM-OFFSETS}.h
    DEPENDS ${CMAKE_SOURCE_DIR}/tools/asm-offsets.sh ${CMAKE_CURRENT_SOURCE_DIR}/asm-offsets.def ${CMAKE_CURRENT_SOURCE_DIR}/object.h ${CMAKE_CURRENT_SOURCE_DIR/}task.h
    VERBATIM
    )

execute_process(
    COMMAND git describe --tags --long
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_SHA1
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in  ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kernel.lds.in ${CMAKE_CURRENT_BINARY_DIR}/kernel.lds @ONLY)
add_executable(none 
    _setup.S
    x86.S
    ${kernel}
    ${mm}
    ${rootfs}
    ${drivers}
    ${input}
    ${CMAKE_CURRENT_BINARY_DIR}/${ASM-OFFSETS}.h)

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
    DESTINATION iso/boot/)
