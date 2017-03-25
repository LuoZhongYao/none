#
#	样板Makefile文件,集体项目根据实际情况进行简单修改即可
#	Author : lzy
.PHONY:all clean

BUILD		:= LIBRARY

src_files := \
	adler32.c \
	compress.c \
	crc32.c \
	deflate.c \
	gzclose.c \
	gzlib.c \
	gzread.c \
	gzwrite.c \
	infback.c \
	inflate.c \
	inftrees.c \
	inffast.c \
	trees.c \
	uncompr.c \
	zutil.c

ifeq ("$(root_dir)","")
	$(error Undefined project root directory, you should run the 'source tools/setenv.sh' in the project root directory.)
else
	include $(root_dir)/tools/Makefile.rule
endif
