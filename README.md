# mrp模拟器在arm linux环境下的实验
 基于 https://github.com/Yichou/mrpoid2018 46694ae

目的是整理出所有系统函数调用简化vmrp的开发

# 编译方法

编译前需要在src/asm文件夹中生成相应的汇编代码 node genR9R10.js isGNU(0|1) isFull(0|1)

例如gcc编译完整版时：
```
node genR9R10.js 1 1
```

例如armcc编译完整版时：
```
node genR9R10.js 0 1
```

## armcc编译vmrp使用的vmrp.mrp(可靠的方式)

需要使用斯凯SDK编译，在src/build文件夹中用build_full.bat编译

## gcc编译vmrp使用的vmrp.elf(不推荐)

需要安装 arm-none-eabi-gcc ，我这里用的是"gcc version 9.3.1 20200408 (release) (GNU Arm Embedded Toolchain 9-2020-q2-update)"

直接在src下make即可

目前还没有找到gcc编译出完全静态的位置无关代码，由于软浮点仍然链接在gcc库中，导致elf中出现GOT表，目前的elfloader过于简单（至少GOT表重定位没有实现），因此gcc编译的方式在实际使用中存在bug

## 在arm linux上运行(由于r9寄存器的问题没解决，所以不推荐)

 需要在arm cpu的linux环境下编译，并且安装libSDL2
# 其它

很多功能被我关闭了，例如网络通信和定时器

mr应该是lua5.0.2，但有更新部分代码到5.0.3


# License

GNU General Public License v3.0
