# C++项目设计文档-γ版

## 流程设计

下图主要展现了对于客户端主线程的流程设计，但是同样的设计方法也运用在服务端的主线程设计当中。在客户端程序中，由于使用了QT框架进行开发，使得当程序从`main()`函数的入口进入程序后，需首先创建一个`QApplication`对象，在这个主线程的流程中`Client`类的对象将作为整个程序的C(Controller)部分用于控制整个程序的运行，`init()`函数作为Client类的成员将对Client对象进行一些基础属性的设置，将其他模块和其进行**组合**连接，而`Run()`函数，将启动程序中的其他模块的线程，并启动线程池。此后的逻辑流程将进入“生产-消费”模式的管理方式，由线程池配合对应的Handler来解决各项任务。

同时鉴于QT自身的消息队列机制，运用其信号和槽，也可以进行任务的分配，但是此处独立于QT的消息机制又做了一套相似的机制的目的是为了和服务端的设计模式相统一，以减少设计成本，单独的进行服务端和客户端的设计对于我来说还是过于复杂。同时这个这个流程设计的过程中，对于之前提到的一些如客户端和服务端之间存在多个连接的设想进行了否决，并对于模块分离设计的MVC模式进行了进一步的细化。



<center>
<img src="C++%E7%AC%AC%E4%B8%89%E6%AC%A1%E5%AE%9E%E9%AA%8C%E6%8A%A5%E5%91%8A/image-20211126231144863.png" alt="image-20211126231144863" style="zoom: 80%;" />
</center>




## UI流程设计

### UI设计图

![image-20211109194428755](C++%E7%AC%AC%E4%B8%89%E6%AC%A1%E5%AE%9E%E9%AA%8C%E6%8A%A5%E5%91%8A/image-20211109194428755.png)

对于整个服务端和客户端的UI使用统一的设计模式，即UI具有管理员模式，其主要结构将和客户端一致，主要多了部分管理员特殊的功能，因为从逻辑上管理员和用户均为系统的用户，只是权限不同。为此要实现这样的设计模式要设计一套统一的API来进行相关操作的调用，并要对用户的权限进行验证。对此也有一套通信协议，将在后文进行说明。

### UI流程
对于UI的流程，主要的设计模式是“生产-消费”模式，即对于用户的触发式操作进行线程的分配。考虑到使用QT界面进行开发，其自身内部隐藏的部分多线程的便利性，可以减少客户端开发过程中的工作量，故而应当对于客户端的ＵＩ进行复用到服务端上。

同时对于UI的逻辑流程，因为是异步的，所以也没有太多的流程可以说明。

## 各模块的设计

### 日志模块

对于日志模块，本想做成WatchDog模式的日志模块，但是考虑到其可移植性，和在服务端的复用性，因为跨平台的存在，致使线程管理的差异，故而并没有设计成原先设想的模式，而是使用了随程序运行过程中对象的生灭机制来实现日志的产出。

### 模块图

![类图](https://s2.ax1x.com/2019/06/17/VHj5Of.png)

在这张图中，Logger作为日志器，是整个模块的中心，LogEvent是日志模块的入口，其他程序通过创建LogEventWarp对象来实现调用LogEvent进而实现对整个模块的调用，而随着作用域的改变被包装的LogEvent对象将被释放从而实现调用，而LogAppender是日志的输出地，即日志的输出位置，或为命令行(Stdout)输出或为文件(File)输出，同时也可以定义其他输出位置，对于日志的格式，为了能使其能够具有一定的自定义性，而定义了格式化器，即LogFormatter，并通过其中的FormatItem实现对于不同格式的处理。

### 网络模块

![plantuml](C++%E7%AC%AC%E4%B8%89%E6%AC%A1%E5%AE%9E%E9%AA%8C%E6%8A%A5%E5%91%8A/plantuml.svg)

对于网络模块，才用的将不同的业务逻辑拆成不同的Handler的方法，Handler将数据和方法进行打包，由Manager将其送入到线程池中进行执行，此外对于Manager的触发，在客户端将主要靠QT的信号和槽的机制，而在服务端，将参考通过线程直接调用函数的方式实现。

### 数据模块

对于数据模块，由于整个程序中所有的数据传递，包括客户端和服务端之间的数据传递将都通过JSON数据结构进行实现，故而对于数据模块来说，关键的数据就只有2种，分别是用户(User)类和物品(Item)类，而商品将是Item类的一个派生类，方便后续添加所谓的优惠券等内容。

### 控制模块

<img src="C++%E7%AC%AC%E4%B8%89%E6%AC%A1%E5%AE%9E%E9%AA%8C%E6%8A%A5%E5%91%8A/plantuml-16379449774411.svg" alt="plantuml" style="zoom:150%;" />

对于控制模块来说，关键是如何进行调度，显然的这里也才用Handler来对业务逻辑进行分割，这里的ClientHandler也是Handler的子类，并下属其他子类用于实现具体的业务逻辑。此外对于Client，应当考虑设计成单例模式。

### 线程模块

对于线程池的设计，目前来说只有一个简单的构想，即需要有一个消息队列和一个线程列表，但对于其如何实现，以及相关的线程调度问题目前尚未有一个明确的思路，但首先想到的是线程池也需要一个线程来进行维护和运行。

## 通信协议的详细设计

首先，通信协议中的报文才用JSON格式的字符串形式进行传输，其主要格式如下

```json
//数据对象
{
    session:"",	//会话的雪花id；
    time:	,//时间戳
    handler:""	//数据包对应的Handler的名称
    type:"",	//数据包类型
    status:200,	//状态id
     contents:[
    ]
}
```
### type的类型

|   类型   |             含义             |
| :------: | :--------------------------: |
| request  | 用于客户端和服务端之间的请求 |
| response | 用于客户端和服务端之间的响应 |
|  heart   |          用于心跳包          |

### status的id

对于状态id，主要存在以下3种状态

|  id  |   含义   |
| :--: | :------: |
| 200  |   正常   |
| 404  |   错误   |
| 206* | 部分正常 |

**\***对于带星号的状态码，可能不会实现，考虑到部分错误情况下的处理逻辑复杂

此外，主要传递的数据将存放在content数组中，其内容主要为JSON对象(Object)类型的字符串

## 数据库详细设计
数据库将会用到的表

|     表名     |                说明                |
| :----------: | :--------------------------------: |
|  user_login  | 存储用户的登录信息和用户的id类别等 |
| user_shopcar |          用户的购物车信息          |
| user_config  |            用户配置文件            |
| user_coupons |           用户优惠券信息           |
|  item_list   |            商品信息存储            |
|  order_list  |        用于存放商品订单信息        |
| *order_history_map | 用于存放商品订单历史的信息分表名和时间的映射 |
| order_history*_\# | 商品订单的历史信息，\#用于分表区分 |
| coupons_list |             优惠券信息             |
|*chat_history_map| 用于聊天记录分表时的表名和时间范围的映射 |
| chat_history*_\# |              聊天记录表，\#用于分表的区分              |

考虑到实现难度，**\***部分的内容可能不会予以实现，或者才用别的方式实现

对于每个表中数据域的设计，需要在程序编写过程中不断调整，考虑到数据库应该为程序服务的原则，提前设计好数据域将导致此后的程序编写束手束脚，虽然在大部分情况下应当对数据域进行提前设计，但是考虑到现在情况的特殊性和个人设计和编程水平的有闲性，对于数据域的设计不能过早的下结论为好。

## 总结

当设计进入细化的过程中，所需要的技术知识储备也越发巨大，没有对一个技术的了解很难对一个程序的各个模块都设计的到位和合适，设计起来令人如同盲人摸象一般，似有非有，若懂非懂的迷茫感，有那样的想法，但是不知道如何去做。总而言之，这种现象的发生是项目经验过少的缘故，和对于程序开发中用到的各种语言特性不熟练不了解的原因。

同时在设计的过程中，迭代和推翻是必要的，同时从另外的角度进行思考也能给设计添加不少成熟的选择，同时设计也应当考虑个人编程能力，否则便如同所谓的“画饼”很难实现。