Lab 3 Writeup
=============

参考: [CS144-Lab3](https://zhuanlan.zhihu.com/p/465922453)

建议多读几遍文档，还是有一些难度的。

payload size需要将SYN和FIN所占的1个byte去掉。

这里没有对每一个发送的segment设置定时器，只给未确认队列中最前面的segment设置了定时器。
