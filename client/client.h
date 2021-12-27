#ifndef __YOLSPC_CLIENT_H__
#define __YOLSPC_CLIENT_H__

#include "handler.h"
#include "qtgui.h"
#include "socket.h"
#include "utils/dataType.h"
#include <QRunnable>
#include <QThreadPool>
#include <map>
#include <queue>
#include <thread>

namespace yolspc {
//前序声明
class SocketController;
class Handler;
class MainWindow;

class ClientController : public std::enable_shared_from_this<ClientController> {
public:
	typedef std::shared_ptr<ClientController> ptr;
	ClientController();
	~ClientController();

	void run();

	Handler *dispatch(const std::string &rawData);
	Handler *dispatch(const std::string &handlerName, const std::string &rawData);
	void submit(Handler *handler);
	std::shared_ptr<SocketController> getSocketController() { return socket_controller; }

	void init();   //初始函数

public:
	//获取用户对象
	User::ptr getUser() { return m_user; }
	//获取主窗口指针
	MainWindow *getMainWindow() { return m_qtgui; }
	//获取商品列表对象
	std::map<int32_t, Item::ptr> &getItemList() { return itemList; }
	//对商品列表的操作
	Item::ptr getItem(const int32_t &id) const {
		return itemList.find(id) == itemList.end() ? nullptr : itemList.find(id)->second;
	}
	void addItem(const Item::ptr item) { itemList.insert(std::make_pair(item->getId(), item)); }
	void removeItem(const int32_t &id) { itemList.erase(id); }
	void resetItemList() { itemList.clear(); }
	//将物品添加到购物车
	void addToShopCar(const Item::ptr &toAddItem, const int32_t amount);
	//从购物车中移除
	void removeFromShopCar(const uint32_t &id);
	void resetShopCar() {
		shopcarList.clear();
		cost_money = 0;
	}
	//获取已经花费的金额
	const double getCostMoney() const { return cost_money; }
	//获取购物车
	std::map<int32_t, Item::ptr> &getShopCar() { return shopcarList; }

private:
	void initGui();       //初始图形界面
	void initHandler();   //初始化Handler

	QThreadPool *thread_pool;                              //线程池
	std::shared_ptr<SocketController> socket_controller;   //网络控制器
	std::map<std::string, std::function<Handler *(const std::string, ClientController *)>> handler_map;

private:
	friend class MainWindow;   //将主界面声明为自己的友元
	//界面
	MainWindow *m_qtgui;                        // client的图形界面
	User::ptr m_user;                           //用户对象
	std::map<int32_t, Item::ptr> itemList;      //商品列表
	std::map<int32_t, Item::ptr> shopcarList;	//购物车列表
	double cost_money;							//要花费的金额
};

}   // namespace yolspc

#endif   //__YOLSPC_CLIENT_H__