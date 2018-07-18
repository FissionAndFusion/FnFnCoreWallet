# FnFn Socket 技术选型



## 概述与定义

作为FnFn Core Wallet的一个通讯组件，**Socket组件**主要是为Core Wallet的外部应用提供一种除REQ-REP（RPC）模式以外的、拥有主动推送能力的通讯功能。消息传输协议将采用二进制形式以提高系统的传输能力和处理能力。本文涉及的技术项目名称如下： [zeromq](http://zeromq.org/intro:read-the-manual)、 [nanomsg](https://nanomsg.org/)、 [libevent](http://libevent.org/)、 [libev](https://github.com/enki/libev)、 [protobuf](https://developers.google.com/protocol-buffers/)、 [thrift](http://thrift.apache.org/)。



## 架构



## Socket通信



## 消息队列



## 消息协议

二进制的消息协议相较于基于文本的协议（xml、json）有以下优点：传输效率更高、处理性能更好，对于高并发、大数据量的环境更有优势。
