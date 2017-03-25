#
#	样板Makefile文件,集体项目根据实际情况进行简单修改即可
#	Author : lzy
.PHONY:all clean
# LIBS 
STATIC_LIBARY :=  posix c
BUILD		  := BINARY
subdir		  := true
ENTRY 		  := 0xc00000
objs 		  := parser.o lexer.o
cleans		  := $(basename $(wildcard *.c))

ifeq ($(root_dir),)
$(error Undefined project root directory, you should run the 'source tools/setenv.sh' in the project root directory.)
else
include $(root_dir)/tools/Makefile.rule
endif
