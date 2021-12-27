#include "handler.h"
#include "utils/datawarpper.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace yolspc {

// SessionIdHandler
SessionIdHandler::SessionIdHandler(const std::string rawData, ClientController *parent)
	: SocketHandler(rawData, parent) {
	socket = m_parent->getSocketController()->getSocket();
	m_name = "SessionId";
	//解析数据
	this->resolverRawData();
}
void SessionIdHandler::run() {
	//获取contents数组
	QJsonValue contents = m_data.value("contents");
	std::string id;
	if (contents.isArray()) {
		QJsonArray contentsArray = contents.toArray();
		if (!contentsArray.empty()) { id = contentsArray.at(0).toString().toStdString(); }
	}
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "设置socketId为:" << id;
	socket->setId(id);   //设置Id
}

void SessionIdHandler::resolverRawData() {
	//解析数据
	QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(m_rawData));
	m_data            = doc.object();   //获取Json对象
}

// UserLoginHandler
UserLoginHandler::UserLoginHandler(const std::string rawData, ClientController *parent)
	: SocketHandler(rawData, parent) {
	//设置Socket
	socket = m_parent->getSocketController()->getSocket();
	//设置Handler名称
	m_name = "UserLogin";
	connect(this, SIGNAL(startAMsg(const QString &)), parent->getMainWindow(), SLOT(slot_user_error(const QString &)));
	connect(this, SIGNAL(updateUserWindow()), parent->getMainWindow(), SLOT(slot_user_update()));
}

void UserLoginHandler::run() {
	this->resolverRawData();   //解析数据
	if (m_data.value("type").toString().toStdString().compare(DataType::request) == 0) {
		//如果是request类型则代表要向服务器发送数据
		YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "向服务器发送数据:" << m_rawData;
		socket->write(m_rawData);   //发送数据
	} else if (m_data.value("type").toString().toStdString().compare(DataType::response) == 0) {
		//如果是respose则是更加conetent内容来决定更新内容
		std::string content = m_data.value("contents").toArray().at(0).toString().toStdString();
		//处理逻辑
		if (content == "NOT_FIND") {
			emit startAMsg("未注册，请先注册");
		} else if (content == "NO_RIGHT_PASSWORD") {
			emit startAMsg("用户名或密码错误");
		} else if (content == "OVER_LOGIN") {
			emit startAMsg("您的账号已经被登录");
		} else {
			QJsonObject the_content(m_data.value("contents").toArray().at(0).toObject());
			QJsonDocument doc(the_content);
			content = doc.toJson().toStdString();
			// YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << content;
			m_parent->getUser()->fromJson(content);
			emit updateUserWindow();
		}
	}
}

void UserLoginHandler::resolverRawData() {
	if (m_rawData.empty()) {
		//如果rawData是空的则代表是客户端发起的事件
		//生成要发送的数据
		DataWarpper datapackage(socket->getId(), m_name, DataType::request);
		datapackage.addContent(m_parent->getUser()->toJson());
		//获取要发送的元字符串
		m_rawData = datapackage.toJson();
		m_data    = datapackage.toJsonObject();
	} else {
		//解析数据包
		QJsonParseError err_rpt;
		QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(m_rawData), &err_rpt);
		if (err_rpt.error != QJsonParseError::NoError) {
			YURZI_LOG_WARN(YURZI_LOG_NAME("Socket")) << "Json解析异常";
			qDebug() << err_rpt.error;
		}
		m_data = doc.object();   //获取Json对象
		// QJsonDocument awa(m_data);
		// YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << awa.toJson().toStdString();
	}
}

// UserLogoutHandler
UserLogoutHandler::UserLogoutHandler(const std::string rawData, ClientController *parent)
	: SocketHandler(rawData, parent) {
	socket = parent->getSocketController()->getSocket();
	m_name = "UserLogout";
}

void UserLogoutHandler::run() {
	this->resolverRawData();
	YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "向服务器发射数据:" << m_rawData;
	socket->write(m_rawData);
}

void UserLogoutHandler::resolverRawData() {
	DataWarpper datapackage(socket->getId(), m_name, DataType::request);
	datapackage.addContent(m_parent->getUser()->toJson());
	m_parent->getUser()->resetDefault();
	m_rawData = datapackage.toJson();
}

// UserRegisterHandler
UserRegisterHandler::UserRegisterHandler(const std::string rawData, ClientController *parent)
	: SocketHandler(rawData, parent) {
	socket = parent->getSocketController()->getSocket();
	m_name = "UserRegister";
	connect(this, SIGNAL(startAMsg(const QString &)), parent->getMainWindow(), SLOT(slot_user_error(const QString &)));
}

void UserRegisterHandler::run() {
	this->resolverRawData();
	if (m_data.value("type").toString().toStdString() == DataType::request) {
		//客户请求事件
		YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "向服务器发送数据:" << m_rawData;
		socket->write(m_rawData);   //发送数据
	} else if (m_data.value("type").toString().toStdString() == DataType::response) {
		//服务端返回事件
		std::string content = m_data.value("contents").toArray().at(0).toString().toStdString();
		if (content == "ACK") {
			emit startAMsg("注册成功");
		} else if (content == "REGISTERED") {
			emit startAMsg("此用户名已经被注册");
		}
	}
}

void UserRegisterHandler::resolverRawData() {
	if (m_rawData.empty()) {
		//如果rawData是空的则代表是客户端发起的事件
		//生成要发送的数据
		DataWarpper datapackage(socket->getId(), m_name, DataType::request);
		datapackage.addContent(m_parent->getUser()->toJson());
		//获取要发送的元字符串
		m_rawData = datapackage.toJson();
		m_data    = datapackage.toJsonObject();
	} else {
		//解析数据包
		QJsonParseError err_rpt;
		QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(m_rawData), &err_rpt);
		if (err_rpt.error != QJsonParseError::NoError) {
			YURZI_LOG_WARN(YURZI_LOG_NAME("Socket")) << "Json解析异常";
			qDebug() << err_rpt.error;
		}
		m_data = doc.object();   //获取Json对象
		// QJsonDocument awa(m_data);
		// YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << awa.toJson().toStdString();
	}
}

// ItemListHandler
ItemListHandler::ItemListHandler(const std::string rawData, ClientController *parent)
	: SocketHandler(rawData, parent) {
	socket = parent->getSocketController()->getSocket();
	m_name = "ItemList";
	connect(this, SIGNAL(refreshItemList()), m_parent->getMainWindow(), SLOT(slot_refresh_items_table()));
}

void ItemListHandler::run() {
	this->resolverRawData();   //数据处理
	if (m_data.value("type").toString().toStdString() == DataType::request) {
		//向服务器发送请求
		YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "向服务器发送数据:" << m_rawData;
		socket->write(m_rawData);   //发送数据
	} else if (m_data.value("type").toString().toStdString() == DataType::response) {
		//从服务收到数据
		QJsonArray contents = m_data.value("contents").toArray();
		//清空商品列表
		m_parent->resetItemList();
		for (size_t i = 0; i < contents.size(); ++i) {
			//遍历数组将数据取出
			QJsonObject obj(contents.at(i).toObject());
			QJsonDocument doc(obj);
			//得到商品对象
			Item::ptr item = std::make_shared<Item>();
			item->fromJson(doc.toJson().toStdString());
			//将商品对象加入商品列表
			m_parent->addItem(item);
		}
		//发送商品更新信号
		emit refreshItemList();
	}
}

void ItemListHandler::resolverRawData() {
	//解析数据
	if (m_rawData.empty()) {
		//若rawData为空则代表向服务器请求
		DataWarpper datapackage(socket->getId(), m_name, DataType::request);
		m_rawData = datapackage.toJson();
		m_data    = datapackage.toJsonObject();
	} else {
		//否则是服务器发来的数据
		//解析数据包
		QJsonParseError err_rpt;
		QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(m_rawData), &err_rpt);
		if (err_rpt.error != QJsonParseError::NoError) {
			YURZI_LOG_WARN(YURZI_LOG_NAME("Socket")) << "Json解析异常";
			qDebug() << err_rpt.error;
		}
		m_data = doc.object();   //获取Json对象
		// QJsonDocument awa(m_data);
		// YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << awa.toJson().toStdString();
	}
}

// BuyHandler
BuyHandler::BuyHandler(const std::string rawData, ClientController *parent)
	: SocketHandler(rawData, parent) {
	socket = parent->getSocketController()->getSocket();
	m_name = "Buy";

	//信号和槽的连接
	connect(this, SIGNAL(updateInfo()), m_parent->getMainWindow(), SLOT(slot_updateAll()));
	connect(this, SIGNAL(startAMsg(const QString)), m_parent->getMainWindow(), SLOT(slot_show_error_msg(const QString &)));
}

void BuyHandler::run() {
	this->resolverRawData();   //数据处理
	if (m_data.value("type").toString().toStdString() == DataType::request) {
		//向服务器发送商品购物车
		if (m_parent->getShopCar().empty()) {
			//购物车为空
			// todo:给个提示？
			return;
		}
		YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << "向服务器发送数据:" << m_rawData;
		socket->write(m_rawData);
	} else if (m_data.value("type").toString().toStdString() == DataType::response) {
		//从服务器收到反馈
		// 1. 检查状态是否为200
		if (m_data.value("status").toInt() == 200) {
			// 2. 若为200则购物正常
			// 2.1 清空本地购物车
			m_parent->resetShopCar();
			//通知窗口刷新
			emit updateInfo();
		} else if (m_data.value("status").toInt() == 206) {
			// 2. 若为206则尚未完全购买成功
			// 2.1 清空原先购物车
			m_parent->resetShopCar();
			//将返回的未成功购买的商品加回购物车
			QJsonArray contents = m_data.value("contents").toArray();
			double retriveMoney = 0;
			for (int i = 0; i < contents.size(); ++i) {
				QJsonDocument doc(contents.at(i).toObject());
				//得到商品对象
				Item::ptr item = std::make_shared<Item>();
				item->fromJson(doc.toJson().toStdString());
				//加回购物车
				m_parent->addToShopCar(item, item->getAmount());
				//返回金额
				retriveMoney += item->getPrice() * item->getAmount();
			}
			//设置用户余额
			m_parent->getUser()->setMoney(retriveMoney + m_parent->getUser()->getMoney());
			//通知窗口刷新
			emit updateInfo();
			//弹出通知窗口
			emit startAMsg("部分商品未成功购买");
		}
	}
}

void BuyHandler::resolverRawData() {
	//解析数据
	if (m_rawData.empty()) {
		//若为空，则代表向服务器请求
		// 1.生成数据包
		DataWarpper datapackage(socket->getId(), m_name, DataType::request);
		// 2. 加入购物车商品
		for (auto it : m_parent->getShopCar()) { datapackage.addContent(it.second->toJson()); }
		// 3. 生成元数据
		m_rawData = datapackage.toJson();
		m_data    = datapackage.toJsonObject();
	} else {
		//若非空则解析服务器发来的回复
		//解析数据包
		QJsonParseError err_rpt;
		QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(m_rawData), &err_rpt);
		if (err_rpt.error != QJsonParseError::NoError) {
			YURZI_LOG_WARN(YURZI_LOG_NAME("Socket")) << "Json解析异常";
			qDebug() << err_rpt.error;
		}
		m_data = doc.object();   //获取Json对象
		// QJsonDocument awa(m_data);
		// YURZI_LOG_INFO(YURZI_LOG_NAME("Socket")) << awa.toJson().toStdString();
	}
}

// UserDelHandler
UserDelHandler::UserDelHandler(const std::string rawData, ClientController *parent)
	: SocketHandler(rawData, parent) {
	socket = parent->getSocketController()->getSocket();
	m_name = "UserDel";
	connect(this,
	        SIGNAL(startAMsg(const QString &, const QString &)),
	        parent->getMainWindow(),
	        SLOT(slot_show_info(const QString &, const QString &)));
}

void UserDelHandler::run() {
	this->resolverRawData();
	if (m_data.value("type").toString().toStdString() == DataType::request) {
		//若是从客户端发送的请求
		std::string content = m_data.value("contents").toArray().at(0).toString().toStdString();
		QJsonDocument doc   = QJsonDocument::fromJson(QByteArray::fromStdString(content));
		QJsonObject user    = doc.object();
		YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "要删除的用户为" << user.value("nickname").toString().toStdString();
		//生成数据包
		User delUser;
		delUser.setNickname(user.value("nickname").toString().toStdString());
		DataWarpper datapackage(socket->getId(), m_name, DataType::request);
		datapackage.addContent(delUser.toJson());
		m_rawData = datapackage.toJson();

		//向服务端发送数据
		socket->write(m_rawData);

	} else if (m_data.value("type").toString().toStdString() == DataType::response) {
		//从服务端发来的请求
		std::string content = m_data.value("contents").toArray().at(0).toString().toStdString();
		if (content == "ACK") {
			//一切正常
			emit startAMsg("已完成", "信息");
		} else if (content == "NOT_FIND") {
			//用户不存在
			emit startAMsg("用户不存在", "错误");
		}
	}
}

void UserDelHandler::resolverRawData() {
	//解析数据包
	QJsonParseError err_rpt;
	QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(m_rawData), &err_rpt);
	if (err_rpt.error != QJsonParseError::NoError) {
		YURZI_LOG_WARN(YURZI_LOG_NAME("Socket")) << "Json解析异常";
		qDebug() << err_rpt.error;
	}
	m_data = doc.object();   //获取Json对象
}

// UserUpdateHandler
UserUpdateHandler::UserUpdateHandler(const std::string rawData, ClientController *parent)
	: SocketHandler(rawData, parent) {
	socket = parent->getSocketController()->getSocket();
	m_name = "UserUpdate";
	connect(this,
	        SIGNAL(startAMsg(const QString &, const QString &)),
	        parent->getMainWindow(),
	        SLOT(slot_show_info(const QString &, const QString &)));
}

void UserUpdateHandler::run() {
	this->resolverRawData();
	if (m_data.value("type").toString().toStdString() == DataType::request) {
		//若是从客户端发送的请求
		std::string content = m_data.value("contents").toArray().at(0).toString().toStdString();
		QJsonDocument doc   = QJsonDocument::fromJson(QByteArray::fromStdString(content));
		QJsonObject user    = doc.object();
		YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "要更新的用户为" << user.value("nickname").toString().toStdString();
		//生成数据包
		User upUser;
		upUser.setNickname(user.value("nickname").toString().toStdString());
		upUser.setMoney(user.value("money").toDouble());
		DataWarpper datapackage(socket->getId(), m_name, DataType::request);
		datapackage.addContent(upUser.toJson());
		m_rawData = datapackage.toJson();

		//向服务端发送数据
		socket->write(m_rawData);

	} else if (m_data.value("type").toString().toStdString() == DataType::response) {
		//从服务端发来的请求
		std::string content = m_data.value("contents").toArray().at(0).toString().toStdString();
		if (content == "ACK") {
			//一切正常
			emit startAMsg("已完成", "信息");
		} else if (content == "NOT_FIND") {
			//用户不存在
			emit startAMsg("用户不存在", "错误");
		}
	}
}

void UserUpdateHandler::resolverRawData() {
	//解析数据包
	QJsonParseError err_rpt;
	QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(m_rawData), &err_rpt);
	if (err_rpt.error != QJsonParseError::NoError) {
		YURZI_LOG_WARN(YURZI_LOG_NAME("Socket")) << "Json解析异常";
		qDebug() << err_rpt.error;
	}
	m_data = doc.object();   //获取Json对象
}

// ItemDelHandler
ItemDelHandler::ItemDelHandler(const std::string rawData, ClientController *parent)
	: SocketHandler(rawData, parent) {
	socket = parent->getSocketController()->getSocket();
	m_name = "ItemDel";
	connect(this,
	        SIGNAL(startAMsg(const QString &, const QString &)),
	        parent->getMainWindow(),
	        SLOT(slot_show_info(const QString &, const QString &)));
}

void ItemDelHandler::run() {
	this->resolverRawData();
	if (m_data.value("type").toString().toStdString() == DataType::request) {
		//若是从客户端发送的请求
		std::string content = m_data.value("contents").toArray().at(0).toString().toStdString();
		QJsonDocument doc   = QJsonDocument::fromJson(QByteArray::fromStdString(content));
		QJsonObject item    = doc.object();
		YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "要删除的商品id为" << item.value("id").toInt();
		//生成数据包
		Item delItem;
		delItem.setId(item.value("id").toInt());
		DataWarpper datapackage(socket->getId(), m_name, DataType::request);
		datapackage.addContent(delItem.toJson());
		m_rawData = datapackage.toJson();

		//向服务端发送数据
		socket->write(m_rawData);

	} else if (m_data.value("type").toString().toStdString() == DataType::response) {
		//从服务端发来的请求
		std::string content = m_data.value("contents").toArray().at(0).toString().toStdString();
		if (content == "ACK") {
			//一切正常
			emit startAMsg("已完成", "信息");
		} else if (content == "NOT_FIND") {
			//用户不存在
			emit startAMsg("用户不存在", "错误");
		}
	}
}

void ItemDelHandler::resolverRawData() {
	//解析数据包
	QJsonParseError err_rpt;
	QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(m_rawData), &err_rpt);
	if (err_rpt.error != QJsonParseError::NoError) {
		YURZI_LOG_WARN(YURZI_LOG_NAME("Socket")) << "Json解析异常";
		qDebug() << err_rpt.error;
	}
	m_data = doc.object();   //获取Json对象
}

// ItemUpdateHandler
ItemUpdateHandler::ItemUpdateHandler(const std::string rawData, ClientController *parent)
	: SocketHandler(rawData, parent) {
	socket = parent->getSocketController()->getSocket();
	m_name = "ItemUpdate";
	connect(this,
	        SIGNAL(startAMsg(const QString &, const QString &)),
	        parent->getMainWindow(),
	        SLOT(slot_show_info(const QString &, const QString &)));
}

void ItemUpdateHandler::run() {
	this->resolverRawData();
	if (m_data.value("type").toString().toStdString() == DataType::request) {
		//向服务器发送请求
		//重新封装数据包
		std::string content = m_data.value("contents").toArray().at(0).toString().toStdString();
		QJsonDocument doc   = QJsonDocument::fromJson(QByteArray::fromStdString(content));
		QJsonObject item    = doc.object();
		YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "要增加商品为" << content;
		DataWarpper datapackage(socket->getId(), m_name, DataType::request);
		datapackage.addContent(content);
		m_rawData = datapackage.toJson();

		socket->write(m_rawData);

	} else if (m_data.value("type").toString().toStdString() == DataType::response) {
		//从服务端发来的请求
		std::string content = m_data.value("contents").toArray().at(0).toString().toStdString();
		if (content == "ACK") {
			//一切正常
			emit startAMsg("已完成", "信息");
		} else if (content == "ID_CONFLICT") {
			//用户不存在
			emit startAMsg("物品ID冲突", "错误");
		} else if (content == "OWNER_UNDEFINE") {
			emit startAMsg("所有者不存在", "错误");
		}
	}




}

void ItemUpdateHandler::resolverRawData() {
	//解析数据包
	QJsonParseError err_rpt;
	QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(m_rawData), &err_rpt);
	if (err_rpt.error != QJsonParseError::NoError) {
		YURZI_LOG_WARN(YURZI_LOG_NAME("Socket")) << "Json解析异常";
		qDebug() << err_rpt.error;
	}
	m_data = doc.object();   //获取Json对象
}

}   // namespace yolspc