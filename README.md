# mrp模拟器在arm linux环境下的实验
 基于 https://github.com/Yichou/mrpoid2018 46694ae

 将来的目的是整理出所有系统函数调用简化vmrp的开发

# 编译方法
需要在arm cpu的linux环境下编译，并且安装libSDL2
1. 精简版（不支持mr）：在src/mr直接`make`,然后在src中`make`
2. 完整版： 在src/mr中`make mr_vm_full`然后在src中`make mrpoid2`


很多功能被我关闭了，例如网络通信和定时器


