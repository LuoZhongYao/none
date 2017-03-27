# Makefile for install.
# Install ask root.

.PHONY: all libs  modules clean go iso

# Directories.
out_dir  	:= out
ramdisk 	:= $(out_dir)/img/ramdisk.img
iso 		:= $(out_dir)/img/none.iso
none 		:= $(out_dir)/bin/none
modules_dir := $(out_dir)/mnt/ramdisk
cdrom 		:= $(out_dir)/cdrom

LIBDIR 		:= lib
KERNELDIR 	:= kernel
MODULESDIR  := modules
DRIVERSDIR	:= drivers
q 			:= -s
Q 			:= @

MAKE = make
RM = rm
$(shell mkdir -p  $(kernel_dir) $(modules_dir))

all : libs $(none) modules bin

$(none) :  libs
	$(Q) $(MAKE) $(q) -C $(KERNELDIR)

modules : libs
	$(Q) $(MAKE) $(q) -C $(MODULESDIR)

bin : lib
	$(Q) $(MAKE) $(q) -C usr/bin

libs :
	$(Q) $(MAKE) $(q) -C $(LIBDIR)

$(ramdisk) : modules
	$(Q) echo "Generating ramdisk..."
	$(Q) mkdir -p $(out_dir)/img
	$(Q) mkdir -p $(out_dir)/ramdisk/dev
	$(Q) mkdir -p $(out_dir)/ramdisk/usr/png
	$(Q) -rm -f $(out_dir)/radisk/bin/*
	$(Q) cp $(out_dir)/bin/ $(out_dir)/ramdisk/ -r
	$(Q) -rm -f -- $(modules_dir)/bin/none
	$(Q) dd if=/dev/zero of=$(ramdisk) bs=1K count=4K
	$(Q) mkfs.minix -2 $(ramdisk)
	$(Q) export LIBGUESTFS_BACKEND="direct"; guestmount --format=raw -a $(ramdisk) -m /dev/sda $(modules_dir)
	$(Q) chmod a+w $(modules_dir)
	$(Q) cp $(out_dir)/ramdisk/* $(modules_dir)/ -r
	$(Q) sync
	$(Q) guestunmount $(modules_dir)

iso : $(none) $(ramdisk)
	$(Q) echo "Building ISO..."
	$(Q) mkdir -p $(cdrom)/boot/grub/
	$(Q) cp tools/grub.cfg $(cdrom)/boot/grub/
	$(Q) cp $(none) $(cdrom)/
	$(Q) cp $(ramdisk) $(cdrom)/ 
	$(Q) grub2-mkrescue -d /usr/lib/grub/i386-pc -o $(iso) $(cdrom)

clean :
	$(Q) $(MAKE) $(q) -C $(KERNELDIR) clean
	$(Q) $(MAKE) $(q) -C $(LIBDIR) clean
	$(Q) $(MAKE) $(q) -C $(MODULESDIR) clean
	$(Q) -rm -rf -- out/
	$(Q) -rm -f -- *.out *.src tags *.swap
