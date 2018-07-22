# FnFn Socket 技术选型



## 概述与定义

作为FnFn Core Wallet的一个通讯组件，**Socket组件**主要是为Core Wallet的外部应用提供一种除REQ-REP（RPC）模式以外的、拥有主动推送能力的通讯功能。消息传输协议将采用二进制形式以提高系统的传输能力和处理能力。本文涉及的技术项目名称如下： [zeromq](http://zeromq.org/intro:read-the-manual)、 [nanomsg](https://nanomsg.org/)、 [libevent](http://libevent.org/)、 [libev](https://github.com/enki/libev)、[boost::aiso](https://www.boost.org/doc/libs/1_67_0/doc/html/boost_asio.html)、 [protobuf](https://developers.google.com/protocol-buffers/)、[msgpack](https://msgpack.org/)、 [thrift](http://thrift.apache.org/)。

zguide: http://zguide.zeromq.org/page:all



## 架构

单一的Core Wallet节点可以通过Socket组件与大于1个Light wallet service实例保持长连接，Core Wallet节点可将自身产生或者接收到的广播数据主动推送至Light wallet service。

以上情况决定了Core Wallet节点需要一定的并发能力，但由于长连接建立，应用之间无需频繁打开关闭连接，因此并发能力相对来说并不十分重要，更重要的是对数据的快速处理能力。数据处理瓶颈可能来源于网络IO层面以及业务本身的处理难度，这就要求Socket组件能够使用非同步的方式处理网络IO并能够对数据进行列队缓存（消息队列）。

![](socket_img/corewallet.jpg)

## Socket通信

### ZeroMQ

#### 通信协议

提供进程内、进程间、机器间、广播等四种通信协议。通信协议配置简单，用类似于URL形式的字符串指定即可，格式分别为inproc://、ipc://、tcp://、pgm://。ZeroMQ会自动根据指定的字符串解析出协议、地址、端口号等信息。

#### Exclusive-Pair

最简单的1:1消息通信模型，可以认为是一个TCP Connection，但是TCP Server只能接受一个连接。数据可以双向流动，这点不同于后面的请求回应模型。

#### Request-Reply

由请求端发起请求，然后等待回应端应答。一个请求必须对应一个回应，从请求端的角度来看是发-收配对，从回应端的角度是收-发对。跟一对一结对模型的区别在于请求端可以是1~N个。该模型主要用于远程调用及任务分配等。Echo服务就是这种经典模型的应用。

REQ/REP 是最基本的模式。客户端发送数据请求服务器的响应。

![](socket_img/fig2.png)

#### Publish-Subscribe

发布端单向分发数据，且不关心是否把全部信息发送给订阅端。如果发布端开始发布信息时，订阅端尚未连接上来，则这些信息会被直接丢弃。订阅端未连接导致信息丢失的问题，可以通过与请求回应模型组合来解决。订阅端只负责接收，而不能反馈，且在订阅端消费速度慢于发布端的情况下，会在订阅端堆积数据。该模型主要用于数据分发。天气预报、微博明星粉丝可以应用这种经典模型。

- 订阅者可以连接到不止一个发布者，每个连接使用一次`connect`调用。订阅者会从不同的连接轮流(“公平队列”)收取数据，不会有单个的发布者被遗漏在外。

- 如果发布者没有与之相连的订阅者，发布者就会简单的把消息丢弃

- 如果使用的是TCP并且订阅者处理得比较缓慢，消息会在发布者一端排队等待处理。在后面的章节中，我们会使用“高水位”来保护发布者队列。

- 从ZeroMQ 3.x 开始，当使用连接协议时（tcp 或 ipc）过滤操作将在发布者一端执行。在ZeroMQ 2.x 所有的过滤操作都在订阅者一端执行。

Pub/Sub 自身组合使用可以解决很多实际问题。比如有很多数据要发布给内部应用和外部应用使用，而外部应用可以访问的数据是内部应用的一个子集。通过组合 Pub/Sub，让其中一个（或者多个）订阅者在收到数据后，过滤出想要对外发布的 topic（或者 channel），然后再重新发布出去，供外网的应用订阅。

![](socket_img/fig4.png)

#### Push-Pull

Server端作为Push端，而Client端作为Pull端，如果有多个Client端同时连接到Server端，则Server端会在内部做一个负载均衡，采用平均分配的算法，将所有消息均衡发布到Client端上。与发布订阅模型相比，推拉模型在没有消费者的情况下，发布的消息不会被消耗掉；在消费者能力不够的情况下，能够提供多消费者并行消费解决方案。该模型主要用于多任务并行。

Push/Pull 的特点是无论是 Push 端还是 Pull 端都可以做 server，bind 到某个地址等待对方访问。如果我们在 Push 端绑定地址，那么这是一个 Push server，对应的 Pull clients 可以 connect 到这个 Push server 往外拉数据；反之，如果我们建立一个 Pull server，对应的 Push clients 可以 connect 到这个 Pull server 往里压数据。由此，我们可以轻松实现一个 task 的 map reduce 的 framework。中间的 worker 可以随需增减。

Push/Pull 模式的另外一个应用场景是 fair queue — Push clients 轮番往 Pull server 写入数据。

![](socket_img/fig5.png)

#### Router-Dealer

Router/Dealer 模式是典型的 broker 模式。在多对多的网络中， 掮客起到在网络的两端双方互不认识的情况下，促成双方的交易。超市就是一个典型的掮客。顾客不必和所有的供应商一一打交道，每个供应商也不需要认识所有的顾客来促成交易 —— 整个交易在超市的促成下完成，双方几乎都不知道对方的存在。多对多的网络中，Router/Dealer 模式很有用。假设我们有 N 个 Reply server，M 个 Request client，若要保证高可用性，正常而言，双方需要一个 M x N 的 full mesh 的网络才能保证任何一个 client 能够和任何一个 server 建立连接。通过在中间加一层 Router/Dealer，M x N 的连接被简化成 M + N。网络的复杂度大大降低。

### Nanomsg

Nanomsg是zeromq的其中一个作者重新开发的下一代类zeromq系统，其中对zeromq实现的一些问题进行了反思，并体现在nanomsg设计中。nanomsg与zeromq不同的地方有：

- POSIX 兼容性的实现 （与zmq不同，nanomsg 目标是保持完全的 POSIX 兼容性）;
- 与使用C++的zmq不同的是，nanomsg使用C实现;
- 更方便扩展的传输协议;
- 线程模型改进;
- IOCP 的支持;
- Routing 优先级的支持;
- 其他改进;

#### 通讯协议

提供了进程内、进程间、机器间的通讯协议，支持inproc、ipc、tcp、websocket。

#### PAIR

简单的一对一通讯。

#### BUS

简单的多对多通讯。

#### REQ-REP

允许构建无状态服务集群以处理用户请求，与zeromq的req-rep模式相同。

#### PUB-SUB

将消息分发给订阅用户，与zeromq的pub-sub模式相同。

#### PIPELINE

聚合来自多个源的消息，并在可以在聚合端进行负载平衡。

#### SURVEY

允许一次查询多个应用程序的状态。

### Libevent&Libev&Boost::asio

Libevent库提供了以下功能：当一个文件描述符的特定事件 （如可读，可写或出错）发生了，或一个定时事件发生了， libevent就会自动执行用户指定的回调函数，来处理事件。libevent已支持了/dev/poll、kqueue(2)、 event ports、select(2)、poll(2) 、epoll(4)等接口，并将io、signal、dns、timer、idle都抽象成为了事件。

Libev与libevent的功能相似，同样也对io等操作进行了事件抽象，提供了非阻塞的io操作能力，nodejs的作者就是在libev的基础上封装了libuv，后者成为了nodejs的事件驱动编程的基础。

Boost::asio是一个跨平台的网络及底层IO的C++编程库，它使用现代C++手法实现了统一的异步调用模型。asio库能够使用TCP、UDP、ICMP、串口来发送/接收数据。

以上三个库，设计目的都在于——针对计算机底层的各种io的进行事件抽象。通过其提供的事件驱动开发能力，可以实现出各种并发（连接）能力不错的平台、客户端与服务器（nodejs、chrome、webserver），但是三个库均没有做更进一步的抽象，这是由其设计目的所决定，并且需要开发者来根据需要自行设计并实现的。



## 消息队列

传统的消息队列中有一个“中央集权式”的messaging broker，该messaging broker通常会负责消息在各个节点之间的传输。而对于zeromq用zguide（**概述与定义**有链接）中的话讲就是：decentralized。zeromq并不要求你的messaging topology中央必须是一个message broker（这个message broker可能作为消息的存储、转发中心）。在一些简单的通信模型中，省去message broker确实为我们省去了很多工作。而且我们也无需为message broker专门搭建一个服务器。

如果缺少了message broker，那么未及发送/接受的消息会不会丢失？通常情况下，zeromq中一些套接字本身自带一个buffer，会把这些消息先存下来。但是zeromq的去中心化不代表完全的去中心化。zeromq把建立message broker的自由交给了我们。这样，我们可以在需要的时候建立一个proxy，来简化网络的复杂性和维护成本。zguide中讲到的The Dynamic Discovery Problem、Shared Queue其实都是在教我们在不同场景下应该怎样建立一个broker来降低网络的复杂性而提升其灵活性。
而且，对于一个复杂的消息拓扑，“各自为政”可能会需要在加入新的节点时重新配置消息拓扑（这会在什么情况下发生，具体可以参考zguide中在介绍The Dynamic Discovery Problem、Shared Queue时引入的例子）。zguide中描述The Dynamic Discovery Problem这个问题时，拿PUB-SUB模式来举例，说明了使用中间件可以降低两两互联网络的维护成本。中间件的引入使网络更加灵活，因而增加新的节点更加简单。如果不采用中间件，则每次增加新的节点时（比如增加一个新的PUB节点），要重新配置该新节点和现有其他节点之间的关系（比如，把刚才新增的PUB节点和所有现有的SUB节点相连）。

## 消息协议

二进制的消息协议相较于基于文本的协议（xml、json）有以下优点：传输效率更高、处理性能更好，对于高并发、大数据量的环境更有优势，缺点是——相对于xml缺乏自描述信息。

### Protobuf

Protocol Buffers 是一种轻便高效的结构化数据存储格式，可以用于结构化数据串行化，或者说序列化。它很适合做数据存储或 RPC 数据交换格式。可用于通讯协议、数据存储等领域的语言无关、平台无关、可扩展的序列化结构数据格式。

使用方法：

- 定义用于消息文件.proto;
- 使用protobuf的编译器编译消息文件;
- 使用编译好对应语言的类文件进行消息的序列化与反序列化;

```protobuf
message Person {
  required string name = 1;
  required int32 id = 2;
  optional string email = 3;
}
```

```java
Person john = Person.newBuilder()
    .setId(1234)
    .setName("John Doe")
    .setEmail("jdoe@example.com")
    .build();
output = new FileOutputStream(args[0]);
john.writeTo(output);
```

```swift
Person john;
fstream input(argv[1],
    ios::in | ios::binary);
john.ParseFromIstream(&input);
id = john.id();
name = john.name();
email = john.email();
```

团队必须共同维护.proto文件，业务发生变化需要更新对应的.proto文件，并且重新生成各个项目下对应的源代码文件，这既是优势也是其缺点——牺牲了灵活性但提高了对个项目开发者的约束。

### Msgpack

MessagePack是一个基于二进制高效的对象序列化类库，可用于跨语言通信。它可以像JSON那样，在许多种语言之间交换结构对象；但是它比JSON更快速也更轻巧。

相较于protobuf其使用更灵活，可以像处理json一样进行序列化与反序列化，可以看作是json的二进制版本。

### Thrift

Thrift比起前二者跟像是通过定义.thrift文件而创建整套RPC服务，二进制消息协议只是其中的一部分，所以将不做分析与说明。

## 总结

Libevent、libev、boost::aiso 这三者的服务端实现较为容易，但是在客户端实现过程中需要处理太多细节问题，如：断线重连、实现高效且稳定的消息推送等。并且服务端需要自行设计消息缓存机制（队列），以防止处理密集数据时有可能产生的各种问题。

ZeroMQ改变TCP基于字节流收发数据的方式，处理了粘包、半包等问题，以msg为单位收发数据，结合ProtoBuf，可以对应用层彻底屏蔽网络通信层。其采用了非阻塞的异步网络IO处理，可以应对大量并发请求，并且通过组合其REQ-REP模式、PUB-SUB模式、PUSH-PULL模式可以满足目前的需求以及以后有可能的需求变更。

本文的选型标准之一是尽量简化构架，所以ZeroMQ这种无第三方Broker的弱消息队列方案比较符合这一预期，二进制消息协议倾向于ProtoBuf，但MsgPack的简单性也有足够的吸引力，可以尝试搭配使用。

Nanomsg脱胎于ZeroMQ，理论上应该比ZeroMQ更优秀，但是目前业界应用比较少，文档稍微欠缺，所以不是一个最优的选择。

| 项目     | 选择结果 |
| -------- | -------- |
| ZeroMQ   | 优先选择 |
| Nanomsg  | 备选     |
| ProtoBuf | 优先选择 |
| MsgPack  | 备选     |

