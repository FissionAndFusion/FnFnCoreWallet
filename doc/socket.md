# FnFn Socket 技术选型



## 概述与定义

作为FnFn Core Wallet的一个通讯组件，**Socket组件**主要是为Core Wallet的外部应用提供一种除REQ-REP（RPC）模式以外的、拥有主动推送能力的通讯功能。消息传输协议将采用二进制形式以提高系统的传输能力和处理能力。本文涉及的技术项目名称如下：zeromq[^1]、nanomsg[^2]、libevent[^3]、libev[^4]、protobuf[^5]、thrift[^6]。



## 架构





## Socket通信





## 消息队列





## 消息协议





二进制的消息协议相较于基于文本的协议（xml、json）有以下优点：传输效率更高、处理性能更好，对于高并发、大数据量的环境更有优势。



zeromq

nanomsg

libevent

libev

protobuf

thrift



[^1]: http://zeromq.org/intro:read-the-manual
[^2]: https://nanomsg.org/
[^3]: http://libevent.org/
[^4]: https://github.com/enki/libev
[^5]: https://www.ibm.com/developerworks/cn/linux/l-cn-gpb/index.html
[^6]: https://www.ibm.com/developerworks/cn/java/j-lo-apachethrift/

[1]: https://www.ibm.com/developerworks/cn/linux/l-cn-gpb/index.html	"Google Protocol Buffer 的使用和原理"
[2]: https://www.ibm.com/developerworks/cn/java/j-lo-apachethrift/	"Apache Thrift - 可伸缩的跨语言服务开发框架"
[3]: https://nanomsg.org/	"About Nanomsg"
[4]: http://zeromq.org/intro:read-the-manual	"ZeroMQ"

