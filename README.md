Clusters Performance Web Monitor
============

A simple clusters performance web monitor with node.js, just for fun.

###### 1、这是什么？
> + 一个简单的集群性能Web监&控应用(仅针对Win Server)
+ 用于监视Windows服务器结点性能(CPU、Memory、Disk IO)
+ 主要基于Windows计数器及Node.js实现

###### 2、有何特点？
> + 支持多受控结点及多客户端
+ 实时图表呈现性能数据(毫秒级响应)
+ 针对Win Server，可对百项性能数据监控
    + 对于Linux平台可使用proc文件系统采集性能数据
+ 支持受控结点远程命令，简易性能报警

###### 3、使用了哪些技术？
> + 前端：jQuery、HighCharts
+ 后端：Node.js、Socket.io、WinSock、Windows Performance Counters

###### 4、如何实现？
> + 实现效果，详细介绍及开发细节请参考：
+ https://docs.google.com/open?id=0B8W2neTuEiYGZDVfN1NxVE5sWTQ
+ https://docs.google.com/open?id=0B8W2neTuEiYGT2dhRUE2SnVCSzQ

###### 5、如何运行？
> + 使用VS编译WebMonitorServer后，在每个需要监控的结点运行
+ 在WebMonitorMiddleware中，node ./main.js 以运行服务端
+ 访问http://localhost/

###### 6、致谢
> + Sublime Text 2