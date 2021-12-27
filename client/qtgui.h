#ifndef __YOLSPC_QTGUI_H__
#define __YOLSPC_QTGUI_H__

#include "utils/dataType.h"
#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPushButton>
#include <client.h>
#include <log.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
class UserWindow;
class ShopCarWindow;
class AdminWindow;
}   // namespace Ui
QT_END_NAMESPACE

namespace yolspc {
//前序声明
class ClientController;
class UserWindow;
class ShopCarWindow;
class AdminWindow;

//主窗口
class MainWindow : public QWidget {
	Q_OBJECT
public:
	MainWindow(QWidget *parent = nullptr, ClientController *_client = nullptr);
	~MainWindow();

	//获取主控制器指针以便于后续操作
	ClientController *getClient() { return client; }

public slots:
	void slot_refresh_items();                          //刷新商品列表
	void slot_refresh_items_table();                    //刷新商品展示界面
	void slot_show_userwindow();                        //展示用户登录界面
	void slot_user_update();                            //用户更新槽函数
	void slot_show_error_msg(const QString &content);   //显示消息窗口
	void slot_show_info(const QString &content, const QString &title = "信息");
	void slot_user_error(const QString &content);        //用于用户界面显示消息窗口
	void slot_show_table_rightMenu(const QPoint &pos);   //用于展示列表右键时的菜单
	void slot_add_to_shopcar();
	void slot_show_shopcar();         //展示购物车界面
	void slot_refresh_cost_label();   //刷新花费金额标签
	void slot_updateAll();
	void slot_on_search();                             //搜索
	void slot_show_adminwindow(bool visible = true);   //展示管理窗口

private:
	ClientController *client;       //控制器指针
	Ui::MainWindow *ui;             //界面元素指针
	UserWindow *userwindow;         //用户登录界面
	ShopCarWindow *shopcarwindow;   //购物车界面
	QMenu *tableRightMenu;          //列表界面的右键菜单
	QAction *addToShopCar;          //添加到购物车的动作
	AdminWindow *adminwindow;       //管理界面

private:
	friend class UserWindow;   //声明友元界面
	friend class ShopCarWindow;
	friend class AdminWindow;
};

//用户登录与注册界面
class UserWindow : public QWidget {
	Q_OBJECT
public:
	UserWindow(QWidget *parent = nullptr);
	~UserWindow();

	void setIcon(const std::string &image) { m_icon = image; }
	void setIsLogin(bool status) { isLogin = status; }
	const bool getIsLogin() { return isLogin; }
	void setUserInfo(User::ptr user);

private slots:

	void slot_enable_registerAndLogin();   //检查文本框以决定注册和登录按钮是否启用
	void slot_on_login_clicked();          //当登录按钮被按下
	void slot_on_register_clicked();       //当注册按钮被按下
	void slot_on_logout_cliecked();        //当退出按钮被按下

signals:
	void userUpdata();

private:
	//绘图事件处理函数
	void paintEvent(QPaintEvent *e) override;

private:
	Ui::UserWindow *ui;                         //界面元素指针
	std::string m_icon = ":/assets/icon.png";   //用户头像地址 默认值
	bool isLogin;                               //是否已经登录
};

class ShopCarWindow : public QWidget {
	Q_OBJECT
public:
	ShopCarWindow(QWidget *parent = nullptr);
	~ShopCarWindow();

public slots:
	void slot_show_rightMenu(const QPoint &);
	void slot_remove_from_shopcar();
	void slot_update_table();
	void slot_on_buy_clicked();   //	清空购物车按钮被按下

private:
	Ui::ShopCarWindow* ui;	//界面元素指针
	QMenu* rightMenu;	//右键菜单
	QAction* removeFromShopCar;	//移除购物车的动作
};

class AdminWindow: public QWidget {
	Q_OBJECT
public:
	AdminWindow(QWidget* parent = nullptr);
	~AdminWindow();
public slots:
	//用户相关的槽函数
	void slot_user_del();
	void slot_user_update();
	void slot_item_del();
	void slot_item_update();

private:
	Ui::AdminWindow* ui;
};

}   // namespace yolspc

#endif   //__YOLSPC_QTGUI_H
