#ifndef __YOLSPC_HANDLER_H__
#define __YOLSPC_HANDLER_H__

#include "client.h"
#include "utils/dataType.h"
#include <QJsonObject>
#include <QRunnable>
#include <QString>
#include <QThread>
#include <memory>

namespace yolspc {
class ClientController;
class Socketer;

//处理者类
class Handler : public QObject, public QRunnable {
	Q_OBJECT
public:
	typedef std::shared_ptr<Handler> ptr;   //指针

	Handler(const std::string rawData, ClientController *parent)
		: m_rawData(rawData)
		, m_parent(parent) {}

	virtual void run() = 0;   //运行函数

	const std::string getHandlerName() const { return m_name; }
	const QJsonObject getData() const { return m_data; }
	void setRawData(const std::string &rawData) { m_rawData = rawData; }   //设置元数据
signals:
	void startAMsg(const QString &msg, const QString &title = "信息");

protected:
	virtual void resolverRawData() = 0;   //数据解析函数

	std::string m_name;           // Handler的名字
	QJsonObject m_data;           // 存放数据
	std::string m_rawData;        // 原始字符串数据
	ClientController *m_parent;   //控制器指针
};

//与网络无关的Handler
class ClientHandler : public Handler {
public:
	typedef std::shared_ptr<ClientHandler> ptr;
	ClientHandler(std::string rawData, ClientController *parent)
		: Handler(rawData, parent) {}
};

//与网络相关的Handler
class SocketHandler : public Handler {
public:
	typedef std::shared_ptr<SocketHandler> ptr;
	SocketHandler(std::string rawData, ClientController *parent)
		: Handler(rawData, parent) {}

protected:
	Socketer *socket;
};

class SessionIdHandler : public SocketHandler {
public:
	SessionIdHandler(const std::string rawData, ClientController *parent);
	void run() override;

private:
	void resolverRawData() override;
};

class UserLoginHandler : public SocketHandler {
	Q_OBJECT
public:
	UserLoginHandler(const std::string rawData, ClientController *parent);
	void run() override;
signals:
	void updateUserWindow();

private:
	void resolverRawData() override;
};

class UserLogoutHandler : public SocketHandler {
public:
	UserLogoutHandler(const std::string rawData, ClientController *parent);
	void run() override;

private:
	void resolverRawData() override;
};

class UserRegisterHandler : public SocketHandler {
public:
	UserRegisterHandler(const std::string rawData, ClientController *parent);
	void run() override;

private:
	void resolverRawData() override;
};

class ItemListHandler : public SocketHandler {
	Q_OBJECT
public:
	ItemListHandler(const std::string rawData, ClientController *parent);
	void run() override;
signals:
	void refreshItemList();

private:
	void resolverRawData() override;
};

class BuyHandler : public SocketHandler {
	Q_OBJECT
public:
	BuyHandler(const std::string rawData, ClientController *parent);
	void run() override;

signals:
	void updateInfo();

private:
	void resolverRawData() override;
};

class UserDelHandler : public SocketHandler {
	Q_OBJECT
public:
	UserDelHandler(const std::string rawData, ClientController *parent);
	void run() override;

private:
	void resolverRawData() override;
};

class UserUpdateHandler : public SocketHandler {
	Q_OBJECT
public:
	UserUpdateHandler(const std::string rawData, ClientController *parent);
	void run() override;

private:
	void resolverRawData() override;
};

class ItemDelHandler : public SocketHandler {
	Q_OBJECT
public:
	ItemDelHandler(const std::string rawData, ClientController *parent);
	void run() override;

private:
	void resolverRawData() override;
};

class ItemUpdateHandler : public SocketHandler {
	Q_OBJECT
public:
	ItemUpdateHandler(const std::string rawData, ClientController *parent);
	void run()override;
private:
	void resolverRawData()override;
};

}   // namespace yolspc

#endif