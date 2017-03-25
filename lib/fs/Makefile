#
#	样板Makefile文件,集体项目根据实际情况进行简单修改即可
#	Author : lzy
.PHONY:all clean
#当前目录
BUILD		:= LIBRARY
src_files   := $(shell find . -name "*.c")
src_files   += $(shell find . -name "*.s")

ifeq ("$(root_dir)","")
$(error Undefined project root directory, you should run the 'source tools/setenv.sh' in the project root directory.)
else
include $(root_dir)/tools/Makefile.rule
endif
