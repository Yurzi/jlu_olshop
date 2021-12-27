#ifndef __YOLSPC_SOCKET_H__
#define __YOLSPC_SOCKET_H__

#include "client.h"
#include <QMutex>
#include <QTcpSocket>
#include <log.h>
#include <memory>
#include <stdint.h>
#include <string>

namespace yolspc {

class ClientController;
class SocketController;

class SocketStatus {
public:
	enum status { IDLE = -1, OK = 0, FAILED = 1, READY = 2 };
};

// socketer 用于创建tcpsokcet连接
class Socketer : public QObject {
	Q_OBJECT

signals:
	//发送信号
	void m_readReady();

public:
	typedef std::shared_ptr<Socketer> ptr;   //指针

	//初始化构造
	Socketer(const std::string &host_name, const uint16_t host_port, SocketController *parent);
	//析构
	~Socketer();
	//连接
	void connectToHost();
	//读写
	void write(const std::string &str);
	const std::string read();
	void read(std::string &str);
	//状态获取
	const SocketStatus::status getStatus() const { return m_status; }
	// Getter&Setter
	const std::string getId() const { return m_id; }
	const std::string getHostName() { return m_host_name; }
	const uint16_t getHostPort() { return m_host_port; }

	void setId(const std::string &id) { m_id = id; }

public slots:
	//定义槽函数
	void slot_onStateChanged(QAbstractSocket::SocketState state);
	void slot_onReadReady();
	void slot_connected();
	void slot_disconnected();

private:
	//远程主机的信息;
	std::string m_host_name;
	uint16_t m_host_port;
	std::string m_id;
	// tcp套接字
	QTcpSocket *m_tcpsocket;
	//状态
	SocketStatus::status m_status;
	//管理者
	SocketController *m_parent;
	QMutex readLock;    //用于Socket读操作的锁
	QMutex writeLock;   //用于Socket写操作的锁
};

class SocketController : public QObject, public std::enable_shared_from_this<SocketController> {
	Q_OBJECT
public:
	typedef std::shared_ptr<SocketController> ptr;
	SocketController(const uint16_t port, std::shared_ptr<ClientController> parent = nullptr);
	SocketController(const std::string &host_ip,
	                 const uint16_t port,
	                 std::shared_ptr<ClientController> parent = nullptr);
	~SocketController();
	//初始化
	void initLog();
	void setParent(std::shared_ptr<ClientController> parent) { m_parent = parent; }
	Socketer *getSocket() const { return m_socketer; }   //获取socket

public slots:
	//派发任务
	void dispatch();

private:
	Socketer *m_socketer;
	std::weak_ptr<ClientController> m_parent;
};
}   // namespace yolspc

#endif