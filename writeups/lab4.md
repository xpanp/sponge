Lab 4 Writeup
=============

[参考](https://kiprey.github.io/2021/11/cs144-lab4)

# DEBUG

## 初始化未激活

由于太多测试用例通不过，我们找一个具体的测试用例进行调试，用如下命令进行测试：

```shell
ctest -R t_ack_rst$ --rerun-failed --output-on-failure
```

可以在`sponge/etc/tests.cmake`中寻找其对应的测试文件。

这里的错误是`TCPConnection`应该一开始就是`active==true`的，而我初始化为了`false`。

## 超时

前面的测试用例都通过以后，从下面这个测试用例开始全部都是超时：

```shell
97/162 Test #104: t_ipv4_client_send ...............***Timeout  10.01 sec
```

根据实验指导文档中的第六节进行调试，发现tcp建立连接的SYN发出去以后，虚拟的tun144网卡根本没有收到数据包，而tun145网卡中，由于客户端没有收到SYN+ACK的数据包，只能不断的重发SYN，经过反复的检查觉得这个问题无解。

后来将代码从云服务器上放到了本地的虚拟机，终于可以继续调试了，这一次终于将所有的测试用例都通过了。

# 结果

通过了所有的测试用例，性能如下：

```shell
CPU-limited throughput                                   : 0.66 Gbit/s
CPU-limited throughput with reordering  : 0.67 Gbit/s
```

同时使用我们自己的tcp也可以跑通`webget`了。

tcp的实验基本做完了，但是从lab3到lab4，太多参考别人的设计了，同时性能也不是很高，网上看到甚至有人优化到3Gbit/s，以后有空应当再好好看看这个实验。