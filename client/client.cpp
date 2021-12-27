#include "client.h"
#include "log.h"
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace yolspc {

ClientController::ClientController() {
	YURZI_LOG_INFO(YURZI_LOG_ROOT()) << "为Client注册日志器...";
	yurzilog::Logger::ptr clientLogger(std::make_shared<yurzilog::Logger>("Client"));
	clientLogger->addAppender(std::make_shared<yurzilog::StdoutLogAppender>());
	clientLogger->addAppender(std::make_shared<yurzilog::FileLogAppender>("log.log"));
	yurzilog::LoggerMgr::GetInstance()->addLogger(clientLogger);

	//初始化用户为游客
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "初始化游客用户";
	m_user = std::make_shared<User>();
}

// 析构函数
ClientController::~ClientController() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "析构控制器...";
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "析构主界面";
	delete m_qtgui;
}

// init
void ClientController::init() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "初始化控制器...";
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "获取线程池...";
	thread_pool = QThreadPool::globalInstance();
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "将线程池最大线程数置为:2";
	thread_pool->setMaxThreadCount(2);
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "启动网络模块...";
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "设置网络模块连接的远程主机为127.0.0.1:11451";
	socket_controller = std::make_shared<SocketController>("127.0.0.1", 11451, shared_from_this());
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "设置游客用户...";
	m_user     = std::make_shared<User>();
	cost_money = 0;   //初始话花费的金额
	initHandler();
	initGui();
}

// Handler Init
void ClientController::initHandler() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "初始化HandlerMap...";
	// ClientHandler
#define XX(str, C)      \
    handler_map.insert( \
        std::make_pair(#str, [](const std::string rawData, ClientController *parent) { return new C(rawData, parent); }))

	XX(SessionId, SessionIdHandler);         //加入SessionIdHandler
	XX(UserLogin, UserLoginHandler);         //加入UserLoginHandler
	XX(UserLogout, UserLogoutHandler);       //加入UserLogoutHandler
	XX(UserRegister, UserRegisterHandler);   //加入UserRegisterHandler
	XX(ItemList, ItemListHandler);           //加入ItemListHandler
	XX(Buy, BuyHandler);                     //加入BuyHandler
	XX(UserDel, UserDelHandler);             //加入UserDelHandler
	XX(UserUpdate, UserUpdateHandler);       //加入UserUpdateHandler
	XX(ItemDel, ItemDelHandler);             //加入ItemDelHandler
	XX(ItemUpdate, ItemUpdateHandler);       //加入ItemUpdateHandler
#undef XX
}

// Gui Init
void ClientController::initGui() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "初始化GUI...";
	m_qtgui = new MainWindow(nullptr, this);
	m_qtgui->show();
}

void ClientController::run() {}

Handler *ClientController::dispatch(const std::string &rawData) {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "根据元数据查找Handler";
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "元数据为" << rawData;
	QJsonDocument document  = QJsonDocument::fromJson(QByteArray::fromStdString(rawData));
	QJsonObject root_obj    = document.object();
	std::string handlertype = root_obj.value("handler").toString().toStdString();
	return dispatch(handlertype, rawData);
}

Handler *ClientController::dispatch(const std::string &handlerName, const std::string &rawData) {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "派发请求...";
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "请求Handler为:" << handlerName;
	auto it = handler_map.find(handlerName);
	if (it != handler_map.end()) {
		YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "找到对应请求";
		Handler *task = it->second(rawData, this);
		return task;
	} else {
		YURZI_LOG_ERROR(YURZI_LOG_NAME("Client")) << "不存在Handler:" << handlerName;
		return nullptr;
	}
}

void ClientController::submit(Handler *handler) {
	if (handler != nullptr) {
		YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "将Handler:" << handler->getHandlerName() << "加入线程池";
		thread_pool->start(handler);
	} else {
		YURZI_LOG_ERROR(YURZI_LOG_NAME("Client")) << "提交了一个空的Handler指针";
	}
}

void ClientController::addToShopCar(const Item::ptr &toAddItem, const int32_t amount) {
	if (toAddItem == nullptr) {
		YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "空指针！";
		return;
	}
	// 1. 查找购物车中是否存在这样的内容
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "ID为:" << toAddItem->getId() << "的商品添加到购物车";
	auto it = shopcarList.find(toAddItem->getId());
	if (it == shopcarList.end()) {
		// 2. 若不存在则将这个商品加入到购物车中
		Item::ptr newItem = std::make_shared<Item>(toAddItem);
		newItem->setAmount(amount);
		shopcarList.insert(std::make_pair(newItem->getId(), newItem));
	} else {
		// 2. 若存在则将该商品的数量合并
		int32_t originalAmount = it->second->getAmount();
		it->second->setAmount(originalAmount + amount);
	}
	cost_money += toAddItem->getPrice() * amount;   //修改总金额的值
}

//从购物车中移除
void ClientController::removeFromShopCar(const uint32_t &id) {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "将ID为:" << id << "的商品从购物车移除";
	auto it = shopcarList.find(id);
	if (it != shopcarList.end()) {
		cost_money = cost_money - (it->second->getPrice() * it->second->getAmount());
		shopcarList.erase(it);
	}
	if (shopcarList.empty()) { cost_money = 0; }
}

}   // end of namespace yolspc