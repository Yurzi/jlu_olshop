#include "socket.h"
#include "utils.h"
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QString>

namespace yolspc {

// Socketer
Socketer::Socketer(const std::string &host_name, const uint16_t host_port, SocketController *parent)
	: m_host_name(host_name)
	, m_host_port(host_port)
	, m_parent(parent) {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "初始化Sokecter...";
	m_status    = SocketStatus::IDLE;
	m_tcpsocket = new QTcpSocket();

	//建立信号和槽的连接
	QObject::connect(m_tcpsocket,
	                 SIGNAL(stateChanged(QAbstractSocket::SocketState)),
	                 this,
	                 SLOT(slot_onStateChanged(QAbstractSocket::SocketState)));
	QObject::connect(m_tcpsocket, SIGNAL(readyRead()), this, SLOT(slot_onReadReady()));
	QObject::connect(m_tcpsocket, SIGNAL(connected()), this, SLOT(slot_connected()));
	QObject::connect(m_tcpsocket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()));
}

Socketer::~Socketer() {
	QObject::disconnect(m_tcpsocket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()));
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "从主机:" << m_host_name << "断开连接";
	m_tcpsocket->disconnectFromHost();
	QObject::destroyed(m_tcpsocket);
	m_tcpsocket = nullptr;
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "删除Socketer";
}

void Socketer::connectToHost() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "连接主机：" << m_host_name << ":" << m_host_port;
	m_tcpsocket->abort();   //清除原有连接
	m_tcpsocket->connectToHost(QString::fromStdString(m_host_name), m_host_port);
	if (!m_tcpsocket->waitForConnected(1000)) {
		m_status = SocketStatus::FAILED;
		YURZI_LOG_WARN(YURZI_LOG_NAME("Socket")) << "与主机:" << m_host_name << ":" << m_host_port << "连接失败";
	}
}

void Socketer::slot_connected() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "主机:" << m_host_name << "连接成功";
	m_status = SocketStatus::OK;
}

void Socketer::slot_onReadReady() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "从主机:" << m_host_name << "收到信息";
	m_status = SocketStatus::READY;
	// std::string tmp;
	// this->read(tmp);
	// YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "从主机收到:" << tmp;
	emit m_readReady();
}

void Socketer::slot_onStateChanged(QAbstractSocket::SocketState state) {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "连接的状态改变了，喵！";
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "现在的状态是:" << m_tcpsocket->state();
}

void Socketer::slot_disconnected() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "与主机:" << m_host_name << "的连接断开了";
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "尝试重连";
	connectToHost();
}

void Socketer::read(std::string &str) {
	str.clear();   //清空字符串
	str = this->read();
}

const std::string Socketer::read() {
	QMutexLocker lock(&readLock);                           //上锁(自动解锁)
	QByteArray buf = m_tcpsocket->read(sizeof(uint32_t));   //读取头部的数据内容
	// YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << buf.length()
	//将长度信息取出
	uint32_t length = buf.toUInt();
	if (GetEndian()) {
		//将大端序转为小端序
		char *p = (char *)&length;
		for (size_t i = 0; i < sizeof(uint32_t); ++i) { p[i] = buf[sizeof(uint32_t) - 1 - i]; }
	}
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "接收到数据长度为:" << length;

	buf.clear();
	buf.resize(length);
	buf      = m_tcpsocket->read(length);   //读取该长度下的数据;
	m_status = SocketStatus::OK;
	return buf.toStdString();   //传出数据
}

void Socketer::write(const std::string &str) {
	QByteArray buf;
	buf.clear();
	uint32_t length = str.size();
	//进行大小端序转换
	if (GetEndian()) { length = LittleToBig(length); }
	buf.append(QByteArray::fromRawData((char *)&length, 4));                              //将数据长度拼接到头部
	buf.append(QByteArray::fromStdString(str));                                           //拼接数据内容
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "向主机:" << m_host_name << "发送数据";   //:" << buf.toStdString();
	QMutexLocker lock(&writeLock);                                                        //自动上锁
	int8_t rt = m_tcpsocket->write(buf);                                                  //发送数据
	lock.unlock();                                                                        //解锁

	if (rt == -1) {
		YURZI_LOG_ERROR(YURZI_LOG_NAME("Socket")) << "向主机:" << m_host_name << ":" << m_host_port << "发送数据时异常";
	}
	if (!m_tcpsocket->waitForBytesWritten(2000)) {
		YURZI_LOG_ERROR(YURZI_LOG_NAME("Socket")) << "向主机:" << m_host_name << ":" << m_host_port << "发送数据时超时";
	}
}

// SokectController

SocketController::SocketController(const uint16_t port, std::shared_ptr<ClientController> parent) {
	if (parent != nullptr) { m_parent = parent; }
	initLog();   //初始化
	//初始化内部的socket
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "初始化内部Socket...";
	m_socketer = new Socketer("127.0.0.1", port, this);
	//将读事件和dispatch相连
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "连接事件...";
	connect(m_socketer, SIGNAL(m_readReady()), this, SLOT(dispatch()));
	//连接到远程服务器
	m_socketer->connectToHost();
}

SocketController::SocketController(const std::string &host_ip, const uint16_t port, std::shared_ptr<ClientController> parent) {
	if (parent != nullptr) { m_parent = parent; }
	initLog();
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "初始化内部Socket...";
	m_socketer = new Socketer(host_ip, port, this);

	//将读事件和dispatch相连
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "连接事件...";
	connect(m_socketer, SIGNAL(m_readReady()), this, SLOT(dispatch()));
	//连接到远程服务器
	m_socketer->connectToHost();
}

SocketController::~SocketController() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "析构网络控制器...";
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "析构Socketer";
	delete m_socketer;   //将内容物析构
}

void SocketController::initLog() {
	//设置日志器
	YURZI_LOG_INFO(YURZI_LOG_ROOT()) << "初始化Socket模块日志器...";
	yurzilog::Logger::ptr socketlogger(std::make_shared<yurzilog::Logger>("Socket"));
	YURZI_LOG_INFO(YURZI_LOG_ROOT()) << "设置Socket模块日志器的输出地...";
	YURZI_LOG_INFO(YURZI_LOG_ROOT()) << "添加标准控制台与文件输出";
	socketlogger->addAppender(std::make_shared<yurzilog::StdoutLogAppender>());
	socketlogger->addAppender(std::make_shared<yurzilog::FileLogAppender>("log.log"));
	YURZI_LOG_INFO(YURZI_LOG_ROOT()) << "使用模块默认输出格式";
	YURZI_LOG_INFO(YURZI_LOG_ROOT()) << "将Socket日志器添加入日志管理器。";
	yurzilog::LoggerMgr::GetInstance()->addLogger(socketlogger);
	YURZI_LOG_INFO(YURZI_LOG_ROOT()) << "Socket模块日志器初始化完成";
}

void SocketController::dispatch() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "向控制器递交请求";
	std::string content = m_socketer->read();
	if (!m_parent.expired()) {
		if (!content.empty()) {
			Handler* handler = m_parent.lock()->dispatch(content);		//向控制器递交请求
			if (handler != nullptr) {
				m_parent.lock()->submit(handler);	//提交任务
			}
		}
	} else {
		YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "未找到父控制器";
	}
}

}   // namespace yolspc.