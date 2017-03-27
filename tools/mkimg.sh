#!/bin/sh

while getopts "o:i:r:" arg
do
    case $arg in
        o) iso=$OPTARG;;
        i) iso_dir=$OPTARG;;
        r) ram_dir=$OPTARG;; 
        *) echo "$0 -o<output.iso> -i<iso dir> -r<ramdisk dir>" 
            exit 1
            ;;
    esac
done
if [ "$ram_dir" != "" ] && [ "$iso" != "" ] && [ "$iso_dir" != "" ];then
    dir=$(mktemp -d)
    mkdir $dir/mnt
    img=$iso_dir/boot/ramdisk.img
    export LIBGUESTFS_BACKEND="direct"
    echo "Generating ramdisk..."
    dd if=/dev/zero of=$img bs=1K count=4K
    mkfs.minix -2 $img
    guestmount --format=raw -a $img -m /dev/sda $dir/mnt
    cp -r $ram_dir/* $dir/mnt/
    sync
    guestunmount $dir/mnt
    grub2-mkrescue -d /usr/lib/grub/i386-pc -o $iso $iso_dir
else
    echo "iso=$iso"
    echo "iso_dir=$iso_dir"
    echo "ram_dir=$ram_dir"
    exit 1
fi
