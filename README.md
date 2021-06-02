# mrp模拟器在arm linux环境下的实验
 基于 https://github.com/Yichou/mrpoid2018 46694ae

目的是整理出所有系统函数调用简化vmrp的开发

# 编译方法

## 编译vmrp使用的vmrp.elf

需要安装 arm-none-eabi-gcc ，我这里用的是"gcc version 9.3.1 20200408 (release) (GNU Arm Embedded Toolchain 9-2020-q2-update)"

## 在arm linux上运行

 需要在arm cpu的linux环境下编译，并且安装libSDL2
# 其它

很多功能被我关闭了，例如网络通信和定时器

mr应该是lua5.0.2，但有更新部分代码到5.0.3


# License

GNU General Public License v3.0
