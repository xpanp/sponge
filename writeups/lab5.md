Lab 5 Writeup
=============

主要是搞清楚arp其实是数据链路层的协议，因此在NetworkInterface中，一个EthernetFrame要么是IP数据报，要么是 ARP报文。ARP报文也只有request和response，相对前面还是简单一些。

另外云服务器同样在测试时有问题，可能模拟的tap网卡某些功能不被云服务器运行吧。
