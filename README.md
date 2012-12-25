DistributedDB
====
这是中科大软件学院2012年秋的网络程序设计课的课程项目，课程主页见：http://219.219.220.231/wiki/NP2012Fall

课程内容

W1-课程准备

Linux开发环境、编辑器、Linux C编程参考资料等
TCP/IP Socket API
完成的标准:编译运行演示client与server实现互相通信的功能，比如打招呼hello、hi。
参考代码server client （gcc source.c -o dest）
提高：封装 - 减少重复代码、局部化外部接口、改善逻辑关系。代码优化方向：简洁、易读、易修改。优化后的代码范例
W2-Nezha Project kickoff：基于Tokyo Cabinet实现key-value数据的存储功能

编译运行Tokyo Cabinet官方提供的测试代码，见 TC系列产品介绍中Tokyo Cabinet部分。
gcc tctest.c -ltokyocabinet
对Tokyo Cabinet的接口进程封装（局部化输入输出）然后实现key-value数据的存储功能，即能通过函数调用读写数据。
编译多个源文件
gcc -c dbapi.c -o dbapi.o
gcc -c main.c -o main.o
gcc -o nezha main.o dbapi.o -ltokyocabinet
遵守 代码编写规范
完成的标准:编译运行演示程序实现key-value数据的存储功能，并进行code review。
范例代码（注意其中main.c和dbapi.h中都没有include TC的头文件）
W3-Milestone 1：实现用命令行方式操作的单机版KV数据库系统

使用scanf读取命令行并解析执行命令，即能通过命令行对KV数据进行存取，如>set 100 hello和>get 100等
使用Makefile来管理和组织代码文件 参考 http://blog.csdn.net/haoel/article/details/2886  http://os.51cto.com/art/200806/75991.htm
git版本控制工具简明使用方法
 《正则表达式教程》 -  web tester
 Vim进阶简明手册
考核标准：
能使用make和make clean进行编译和清理代码
能用命令行实现对数据库的open、close、get、set和delete操作
code review要求W2的封装、cmdline的简洁和其他代码规范要求
W4-给KV数据库系统增加C/S方式的远程访问功能

实现KV数据库系统的远程访问，具体命令行操作功能不变.即编译生成2个可执行文件，一个是服务器，另一个是客户端，服务器在一台机器上，客户端在另一台机器上，在客户端可以显示命令行并接受open、close、get、set和delete操作命令（像单机版一样）。
C/S之间的通信协议设计
软件代码重构，尽可能多地重用已有的代码
考核标准 - 文档为纲、接口为领、测试为要。
实现客户端远程操作数据库的功能，功能同Milestone 1；
支持单元测试和自动化回归测试
code review，代码的模块化
简要明了的文档
W5-Milestone 2：支持多客户和大并发量请求

多线程编程pthread -  pthread代码例子 -  pthread的读写锁
pthread课堂演示案例：pthreaddemo.c
 IO复用技术 -  epoll -  epoll代码例子 -  epoll events
epoll课堂演示案例：server.c - client.c
充分考虑服务器硬件的多CPU多核并发特性和传统CPU的分时复用，最大限度发挥服务器硬件潜能。
 并发服务器
考核标准：
支持多客户端并发访问
仅使用多线程或io复用来实现为及格，能综合利用多线程和io复用以及线程之间分工协作消息通信机制的为优秀
W6-在客户端实现分布式策略

根据key计算出分布式节点的编号，比如有N个节点，任意一个key取key%N即可得到该key存储在哪个节点上，并直接到该节点处做get/set/delete操作。
最终演示方法：1）启动N个服务器节点（应该分布在不同机器上或虚拟机上）；2）启动一个或多个客户端；3)每次get/set/delete操作系统会自动选择某个节点实现数据库访问
这样单个节点上的数据库实际上只是完整数据的一部分，换句话说整个数据库分布存储在各节点上，所以叫分布式数据库。
