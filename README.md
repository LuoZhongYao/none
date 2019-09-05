[源码]: https://github.com/LuoZhongYao/none
[none]: https://github.com/LuoZhongYao/none
[GITHUB]: https://github.com/LuoZhongYao/none
[lzy]: mail:LuoZhongYao@gmail.com

![Build Status](https://travis-ci.org/LuoZhongYao/none.svg?branch=master)

![None新特性预览](http://r.photo.store.qq.com/psb?/2f055629-26d1-450a-b719-0c9a5862abb4/8f5clD6S9VIPUSTIu8Oa52PZh58NjPIIgMlTow0Hi6A!/o/dMaLGsdKEAAA&bo=oAWEA6AFhAMCACQ!&t=5&rf=viewer_311)

<h2 id="overview">概述</h2>

<h3 id='prepare'>准备</h3>

```bash
sudo apt-get install bochs
```
Fedora下面安装

```bash
sudo yum install bochs
```

通过git获取none

```bash
git clone https://github.com/LuoZhongYao/none.git
```

<h3 id='run'>运行</h3>

`out/img/none.iso`,`out/img/ramdisk.img`是已经编译好的的启动镜像(可启动光盘镜像)和ramdisk镜像,直接可以在bochs下运行.安装好bochs后使用如下命令即可运行
```bash
bochs
```

<h2 id="compile">编译none</h2>

```bash
git clone https://github.com/LuoZhongYao/none.git # 下载源码
cd none                 # 进入none源码目录
source tools/setenv.sh  # 设置root_dir环境变量
make                    # 编译生成out/bin/none,及modules/目录下的演示,测试程序,模块,生成的二进制文件在out/bin/目录下
make iso                # 将out/bin/non拷贝到out/cdrom/目录下,将out/bin/下面的程序,拷贝到out/img/ramdisk.img的/bin/目录下,并生成iso镜像,需要输入两次root密码
```
<h3 id="pc">物理机</h3>

如果安装的是linux系统,运行[none][]很简单.将out/bin/none和out/img/ramdisk.img拷贝到/boot/目录下,如果是grub 2.0,编辑/boot/grub2/grub.cfg,在末尾加入

```bash
menuentry 'none OS' {
    load_video
    insmod part_msdos
    insmod minix2
    search --file /none --set root
    multiboot /none
    module    /ramdisk.img
}
```
重新开机,在grub2启动菜单选择none OS即可运行none.
