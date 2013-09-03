# Makefile for install.
# Install ask root.
#
.PHONY:go all clean install reinstall

# Directories.
# root directories
r = $(PWD)
d = a.img
h = n.hd
s = kernel/none

boot = mnt/boot
hw = mnt/hw

FSDIR = fs
COMMONDIR = common
KERNELDIR = kernel
TESTSDIR = tests/
MMDIR = mm
EXECS = shell test date
SUBDIRS = $(COMMONDIR) $(FSDIR) $(MMDIR) $(KERNELDIR)\
		  $(TESTSDIR) 

MAKE = make
RM = rm

all:
	@for dir in  $(SUBDIRS);do\
		$(MAKE) -C $$dir  r=$r || exit 1;\
	done

clean:
	@for dir in  $(SUBDIRS);do\
		$(MAKE) -C $$dir r=$r clean || exit 1;\
	done
	@-rm -f lib/lib*
	@-rm -f kernel/none

debug:
	@objdump -d $r/kernel/none > t.src

tar: 
	@mount -o loop -t ext2 $d $(boot)
#	@mount $h $(hw)
	@chmod a+w $(hw) $(boot)
	@(cd $(TESTSDIR);for exec in $(EXECS);do\
		cp $$exec ../$(hw);\
	done;)
	@sleep 1

stop:
	@umount $(boot)
#	@umount $(hw)

go: tar stop
	@mount -o loop -t ext2 $d $(boot)
	@cp $s $(boot)
	@sleep 1
	@umount $(boot)
	@bochs
