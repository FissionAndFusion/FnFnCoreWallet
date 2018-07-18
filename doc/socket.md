# FnFn Socket 技术选型



## 概述与定义

作为FnFn Core Wallet的一个通讯组件，**Socket组件**主要是为Core Wallet的外部应用提供一种除REQ-REP（RPC）模式以外的、拥有主动推送能力的通讯功能。消息传输协议将采用二进制形式以提高系统的传输能力和处理能力。本文涉及的技术项目名称如下： [zeromq](http://zeromq.org/intro:read-the-manual)、 [nanomsg](https://nanomsg.org/)、 [libevent](http://libevent.org/)、 [libev](https://github.com/enki/libev)、 [protobuf](https://developers.google.com/protocol-buffers/)、 [thrift](http://thrift.apache.org/)。



## 架构



## Socket通信

### ZeroMQ

#### 通信协议

提供进程内、进程间、机器间、广播等四种通信协议。通信协议配置简单，用类似于URL形式的字符串指定即可，格式分别为inproc://、ipc://、tcp://、pgm://。ZeroMQ会自动根据指定的字符串解析出协议、地址、端口号等信息。

#### Exclusive-Pair

最简单的1:1消息通信模型，可以认为是一个TCP Connection，但是TCP Server只能接受一个连接。数据可以双向流动，这点不同于后面的请求回应模型。

#### Request-Reply

由请求端发起请求，然后等待回应端应答。一个请求必须对应一个回应，从请求端的角度来看是发-收配对，从回应端的角度是收-发对。跟一对一结对模型的区别在于请求端可以是1~N个。该模型主要用于远程调用及任务分配等。Echo服务就是这种经典模型的应用。

#### Publish-Subscribe

  发布端单向分发数据，且不关心是否把全部信息发送给订阅端。如果发布端开始发布信息时，订阅端尚未连接上来，则这些信息会被直接丢弃。订阅端未连接导致信息丢失的问题，可以通过与请求回应模型组合来解决。订阅端只负责接收，而不能反馈，且在订阅端消费速度慢于发布端的情况下，会在订阅端堆积数据。该模型主要用于数据分发。天气预报、微博明星粉丝可以应用这种经典模型。

#### Push-Pull

Server端作为Push端，而Client端作为Pull端，如果有多个Client端同时连接到Server端，则Server端会在内部做一个负载均衡，采用平均分配的算法，将所有消息均衡发布到Client端上。与发布订阅模型相比，推拉模型在没有消费者的情况下，发布的消息不会被消耗掉；在消费者能力不够的情况下，能够提供多消费者并行消费解决方案。该模型主要用于多任务并行。



## 消息队列



## 消息协议

二进制的消息协议相较于基于文本的协议（xml、json）有以下优点：传输效率更高、处理性能更好，对于高并发、大数据量的环境更有优势。

### Protobuf

Protocol Buffers 是一种轻便高效的结构化数据存储格式，可以用于结构化数据串行化，或者说序列化。它很适合做数据存储或 RPC 数据交换格式。可用于通讯协议、数据存储等领域的语言无关、平台无关、可扩展的序列化结构数据格式。



## 总结

ZeroMQ改变TCP基于字节流收发数据的方式，处理了粘包、半包等问题，以msg为单位收发数据，结合Protocol Buffers，可以对应用层彻底屏蔽网络通信层。

