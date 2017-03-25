#
#	样板Makefile文件,集体项目根据实际情况进行简单修改即可
#	Author : lzy
.PHONY:all clean
# LIBS 
STATIC_LIBARY := glib posix c zlib png
BUILD		  := BINARY
subdir		  := true
ENTRY 		  := 0xc00000
module_name	  := pv
#LDFLAGS	 	  := -e main
include_dir   += $(root_dir)/libs/glib/include\
				 $(root_dir)/libs/libpng\
				 $(root_dir)/libs/zlib
CLEANS		  := $(basename $(wildcard *.c))

ifeq ($(root_dir),)
$(error Undefined project root directory, you should run the 'source tools/setenv.sh' in the project root directory.)
else
include $(root_dir)/tools/Makefile.rule
endif
