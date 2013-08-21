None 实验性质的操作系统
=======================
>None是一个业余时间来编写的一个实验性质的操作系统。目标是实现一个一切都是对象的操作系统（这个概念来自Unix）。

##名称由来
>None一开始叫做Ant，之后，由于对之前的代码深恶痛绝，一切都被我推翻了重新写过。新的内核被命名gmL。该名字命名的内核是一个Linux和Minix杂交的产物。而且当时对许多概念的不了解，整个内核十分奇怪。当然内核并没有最终能够发布。该内核托管在Google Code。后来受Java的影响，我像将内核设计运行在虚拟机之上，该设想的内核被命名为vmgmL，该内核事实上没有实现虚拟机，而是实现了个丑陋的IP协议（没有TCP）。该代码也托管在Google Code。之后我又有了一些新的想法，之前的代码又被推翻。因为之前做了那么多事，命了这么多名。却都没有发布，所以新的想法被命名为none（名称来自JavaScript）。表示什么也没有,之前的想法都被推翻了。

##当前实现：
-多进程（多对象）
-进程通信（调方法）
-内存管理（这部分不太满意）
-文件系统（同上）
-一个奇特的shell（由我的同事YangYongHai实现）
-等等一些尚未完成的模块。

##下一部计划
-分页内存管理（这部分太重要了，很多模块都都因为这个模块没实现卡住）
-将文件系统实现为Object
-代码生成代码（Lisp盅惑）


##目标
VIM能够顺利跑在None上
