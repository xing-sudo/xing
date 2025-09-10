# makefile文件中，保存了编译器和链接器的参数选项，并且描述了所有源文件之间的关系。
# make程序会读取makefile文件中的数据，然后根据规则调用编译器，汇编器，链接器产生最后的输出。
# 根据makefile的功能理解，A选项是正确的
# Makefile里主要包含了五个东西：显式规则、隐晦规则、变量定义、文件指示和注释， B选项是正确的
# 显式规则说明了，如何生成一个或多个目标文件。
# make有自动推导的功能，所以隐晦的规则可以让我们比较粗糙地简略地书写makefile，
# 比如源文件与目标文件之间的时间关系判断之类
# 在makefile中可以定义变量，当makefile被执行时，其中的变量都会被扩展到相应的引用位置上，
# 通常使用 $(var) 表示引用变量
# 文件指示。包含在一个makefile中引用另一个makefile，类似C语言中的include； 根据这一项可以推导D选项是错误的。
# 注释，makefile中可以使用 # 在行首表示行注释
# 默认的情况下，make命令会在当前目录下按顺序找寻文件名为“GNUmakefile”、“makefile”、“Makefile”的文件
# PHONY表明依赖文件的依赖方法总是被执行，忽略时间对比机制，没有.PHONY的依赖文件的依赖方法只执行一次
# 且有时间对比机制依赖文件没有被修改就不会再执行该依赖方法
# make会自动查看目标文件的依赖方法是否存在，存在直接执行，不存在会将当前依赖方法入栈直到找到依赖方法为止
.PHONY:all 
all:client server
client:client.cc
	g++ -o $@ $^ -std=c++11
server:server.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f client server