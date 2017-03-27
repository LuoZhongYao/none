cmake_minimum_required (VERSION 3.2)

project (none)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

set(CMAKE_ASM-ATT_FLAGS "--32")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -Wl,--build-id=none -std=c11 -fno-builtin -nostdinc -Wall -Werror -fno-stack-protector -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -Wl,--build-id=none -std=c11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-builtin -nostdinc -Wall -Werror")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-asynchronous-unwind-tables -fno-unwind-tables")

set(CMAKE_C_FLAGS_DEBUG "-g" )
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib")

add_definitions(
    -D__NONE__=1
    )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# Configure CCache if available

include_directories(include)
link_directories(${CMAKE_BINARY_DIR}/lib)

file(GLOB_RECURSE cmakelists RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CMakeLists.txt)
foreach(sub ${cmakelists})
    get_filename_component(dir ${sub} DIRECTORY)
    if(dir)
        add_subdirectory(${dir})
    endif()
endforeach()
