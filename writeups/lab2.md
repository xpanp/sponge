Lab 2 Writeup
=============

依赖libpcap-dev库需要安装
```bash
sudo apt install libpcap-dev
```

注意看文档，理解seqno、absolute seqno、stream index的关系。

可能之前有丢包或者延迟了，因此收到FIN之后，并没有真正结束，还需要继续等待，直到所有包都组装完成。