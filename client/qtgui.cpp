#include "qtgui.h"
#include "ui_adminwindow.h"
#include "ui_mainwindow.h"
#include "ui_shopcarwindow.h"
#include "ui_userwindow.h"

#include "utils/datawarpper.h"
#include <QDebug>
#include <QImage>
#include <QMessageBox>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QStandardItemModel>
#include <iomanip>

namespace yolspc {

// mainWindow
MainWindow::MainWindow(QWidget *parent, ClientController *_client)
	: QWidget(parent)
	, ui(new Ui::MainWindow)
	, client(_client) {

	ui->setupUi(this);
	userwindow    = new UserWindow(this);      //初始化用户界面
	shopcarwindow = new ShopCarWindow(this);   //初始化购物车界面
	adminwindow   = new AdminWindow(this);     //初始管理界面
	// userwindow->show();
	//初始化商品展示列表
	ui->table_content->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->table_content->setContextMenuPolicy(Qt::CustomContextMenu);
	QStandardItemModel *model = new QStandardItemModel(0, 5, this);
	model->setHorizontalHeaderItem(0, new QStandardItem(QString("ID")));
	model->setHorizontalHeaderItem(1, new QStandardItem(QString("名称")));
	model->setHorizontalHeaderItem(2, new QStandardItem(QString("价格")));
	model->setHorizontalHeaderItem(3, new QStandardItem(QString("库存")));
	model->setHorizontalHeaderItem(4, new QStandardItem(QString("商家")));
	ui->table_content->setModel(model);
	ui->table_content->setColumnWidth(0, 50);
	ui->table_content->setColumnWidth(1, 600);
	ui->table_content->setColumnWidth(2, 60);
	ui->table_content->setColumnWidth(3, 60);
	ui->table_content->setColumnWidth(4, 151);
	//初始化列表界面右键菜单
	tableRightMenu = new QMenu;
	addToShopCar   = new QAction("添加到购物车");
	tableRightMenu->addAction(addToShopCar);

	//信号和槽的链接
	connect(ui->btn_user, SIGNAL(clicked()), this, SLOT(slot_show_userwindow()));
	connect(userwindow, SIGNAL(userUpdata()), this, SLOT(slot_user_update()));
	connect(ui->btn_refresh, SIGNAL(clicked()), this, SLOT(slot_refresh_items()));
	connect(ui->table_content,
	        SIGNAL(customContextMenuRequested(const QPoint &)),
	        this,
	        SLOT(slot_show_table_rightMenu(const QPoint &)));
	connect(addToShopCar, SIGNAL(triggered()), this, SLOT(slot_add_to_shopcar()));
	connect(ui->btn_shopCar, SIGNAL(clicked()), this, SLOT(slot_show_shopcar()));
	connect(ui->btn_home, SIGNAL(clicked()), this, SLOT(slot_refresh_items_table()));
	connect(ui->btn_search, SIGNAL(clicked()), this, SLOT(slot_on_search()));
	connect(ui->btn_admin, SIGNAL(clicked()), this, SLOT(slot_show_adminwindow()));
}

MainWindow::~MainWindow() {
	//析构界面
	delete ui;
}

void MainWindow::slot_show_userwindow() {
	//检查用户登录情况,并设置登录窗口状态
	if (client->getUser()->getPrivilege() == 0) {
		userwindow->setIsLogin(false);
	} else {
		userwindow->setIsLogin(true);
		userwindow->setUserInfo(client->getUser());
	}

	//获取父窗口界面坐标
	QPoint globalPos = this->mapToGlobal(QPoint(0, 0));                               //父窗口绝对坐标
	int x            = globalPos.x() + (this->width() - userwindow->width()) / 2;     // x坐标
	int y            = globalPos.y() + (this->height() - userwindow->height()) / 2;   // y坐标
	userwindow->move(x, y);
	userwindow->show();
}

void MainWindow::slot_user_update() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("client")) << "更新用户相关界面";
	User::ptr usr = client->getUser();
	if (usr->getPrivilege() == 0) {
		userwindow->setIsLogin(false);
		ui->btn_admin->setEnabled(false);
		ui->lb_money->setText("0.00");
		slot_show_adminwindow(false);
	} else {
		userwindow->setIsLogin(true);
		userwindow->setUserInfo(usr);
		if (usr->getPrivilege() >= User::Privilege::root) { ui->btn_admin->setEnabled(true); }
		//小数位数转换
		char *toConv;
		toConv = new char[400];
		sprintf(toConv, "%.2lf", usr->getMoney());
		ui->lb_money->setText(toConv);
		delete[] toConv;
	}
}

void MainWindow::slot_refresh_items() {
	//刷新按钮触发后的结果
	//调用ItemListHandler
	std::string rawData;
	Handler *task = client->dispatch("ItemList", rawData);
	client->submit(task);
}

void MainWindow::slot_refresh_items_table() {
	//外界触发的商品显示列表的刷新
	//获取内容模型
	QStandardItemModel *model = (QStandardItemModel *)ui->table_content->model();
	// 1.清空Table控件
	for (int32_t i = model->rowCount() - 1; i >= 0; --i) { model->removeRow(i); }
	// 2.将商品内容加入
	for (auto it : client->getItemList()) {
		uint32_t rowCount = model->rowCount();
		Item::ptr item    = it.second;
		//生成对于的Item
		QStandardItem *i0 = new QStandardItem(QString::number(item->getId()));
		QStandardItem *i1 = new QStandardItem(QString::fromStdString(item->getName()));
		QStandardItem *i2 = new QStandardItem(QString::number(item->getPrice()));
		QStandardItem *i3 = new QStandardItem(QString::number(item->getAmount()));
		QStandardItem *i4 = new QStandardItem(QString::fromStdString(item->getOwner()));
		//将item加入到列中
		model->setItem(rowCount, 0, i0);
		model->setItem(rowCount, 1, i1);
		model->setItem(rowCount, 2, i2);
		model->setItem(rowCount, 3, i3);
		model->setItem(rowCount, 4, i4);
	}
	this->update();   //	重绘窗口
}

void MainWindow::slot_add_to_shopcar() {
	// 1.检查是否已经登录
	if (userwindow->getIsLogin()) {
		// 2. 已经登录则获取对于行的商品id
		QStandardItemModel *model = (QStandardItemModel *)ui->table_content->model();
		QModelIndexList selection = ui->table_content->selectionModel()->selectedRows();
		QModelIndex qi;
		int32_t id;
		for (int i = 0; i < selection.count(); ++i) {
			//遍历所有选中的行
			qi = model->index(selection[i].row(), 0);
			id = model->data(qi).toInt();
			//获取到对应的id 后将内容添加到购物车中
			client->addToShopCar(client->getItem(id), 1);
		}
		slot_refresh_cost_label();
		//通知购物车界面
		shopcarwindow->slot_update_table();
	} else {
		// 2. 未登录则弹出提示框
		slot_show_error_msg("未登录，请先登录");
	}
}

void MainWindow::slot_show_table_rightMenu(const QPoint &pos) {
	tableRightMenu->exec(QCursor::pos());
}

void MainWindow::slot_show_shopcar() {
	//获取父窗口界面坐标
	QPoint globalPos = this->mapToGlobal(QPoint(0, 0));                            //父窗口绝对坐标
	int x            = globalPos.x() + this->width();                              // x坐标
	int y            = globalPos.y() + this->height() - shopcarwindow->height();   // y坐标
	shopcarwindow->move(x, y);
	shopcarwindow->show();
}

void MainWindow::slot_refresh_cost_label() {
	//小数位数转换
	char *toConv;
	toConv = new char[400];
	sprintf(toConv, "%.2lf", client->getCostMoney());
	ui->lb_cost->setText(toConv);
	delete[] toConv;
}

void MainWindow::slot_show_error_msg(const QString &content) {

	QMessageBox::information(this, "错误", content);
}

void MainWindow::slot_user_error(const QString &content) {
	QMessageBox::information(userwindow, "错误", content);
}

void MainWindow::slot_show_info(const QString &content, const QString &title) {
	QMessageBox::information(this, title, content);
}

void MainWindow::slot_updateAll() {
	// 1.刷新商品列表
	slot_refresh_items();
	// 2.刷新用户
	//小数位数转换
	char *toConv;
	toConv = new char[400];
	sprintf(toConv, "%.2lf", client->getUser()->getMoney());
	ui->lb_money->setText(toConv);
	delete[] toConv;
	// 3.刷新购物车相关
	slot_refresh_cost_label();
	shopcarwindow->slot_update_table();
}

void MainWindow::slot_on_search() {
	//获取输入框信息
	std::string pattern = ui->ipt_searchLine->text().toStdString();
	if (pattern.empty()) { return; }

	//展示内容
	//获取内容模型
	QStandardItemModel *model = (QStandardItemModel *)ui->table_content->model();
	// 1.清空Table控件
	for (int32_t i = model->rowCount() - 1; i >= 0; --i) { model->removeRow(i); }
	// 2.将商品内容加入
	for (auto it : client->getItemList()) {
		uint32_t rowCount = model->rowCount();
		Item::ptr item    = it.second;
		if (pattern != std::to_string(item->getId()) && pattern != item->getName() && pattern != item->getOwner()) { continue; }
		//生成对于的Item
		QStandardItem *i0 = new QStandardItem(QString::number(item->getId()));
		QStandardItem *i1 = new QStandardItem(QString::fromStdString(item->getName()));
		QStandardItem *i2 = new QStandardItem(QString::number(item->getPrice()));
		QStandardItem *i3 = new QStandardItem(QString::number(item->getAmount()));
		QStandardItem *i4 = new QStandardItem(QString::fromStdString(item->getOwner()));
		//将item加入到列中
		model->setItem(rowCount, 0, i0);
		model->setItem(rowCount, 1, i1);
		model->setItem(rowCount, 2, i2);
		model->setItem(rowCount, 3, i3);
		model->setItem(rowCount, 4, i4);
	}
	this->update();   //	重绘窗口
}

void MainWindow::slot_show_adminwindow(bool visible) {
	//获取父窗口界面坐标
	QPoint globalPos = this->mapToGlobal(QPoint(0, 0));                                //父窗口绝对坐标
	int x            = globalPos.x() + (this->width() - adminwindow->width()) / 2;     // x坐标
	int y            = globalPos.y() + (this->height() - adminwindow->height()) / 2;   // y坐标
	if (visible) {
		adminwindow->move(x, y);
		adminwindow->show();
	} else {
		adminwindow->hide();
	}
}

// userWindow
UserWindow::UserWindow(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::UserWindow) {
	ui->setupUi(this);

	isLogin = false;   //初始化登录标识符为false;

	//设置窗口属性
	this->setWindowFlag(Qt::Dialog);
	this->setWindowModality(Qt::WindowModal);

	//连接信号和槽
	connect(ui->input_usrname, SIGNAL(textChanged(QString)), this, SLOT(slot_enable_registerAndLogin()));
	connect(ui->input_passwd, SIGNAL(textChanged(QString)), this, SLOT(slot_enable_registerAndLogin()));
	connect(ui->btn_login, SIGNAL(clicked()), this, SLOT(slot_on_login_clicked()));
	connect(ui->btn_register, SIGNAL(clicked()), this, SLOT(slot_on_register_clicked()));
	connect(ui->btn_logout, SIGNAL(clicked()), this, SLOT(slot_on_logout_cliecked()));
}

UserWindow::~UserWindow() {
	delete ui;
}

void UserWindow::setUserInfo(User::ptr user) {
	//设置登录窗口按钮状态
	ui->btn_register->setEnabled(false);
	ui->btn_login->setEnabled(false);
	ui->btn_logout->setEnabled(true);
	//设置文本框内容
	ui->lb_status->setText("已登录");
	ui->lb_privilege->setText(QString::fromStdString(user->getPrivilegeString()));
	ui->input_usrname->setEnabled(false);
	ui->input_passwd->setEnabled(false);
	ui->input_usrname->setText(QString::fromStdString(user->getNickname()));
	ui->input_passwd->setText(QString::fromStdString(user->getPassword()));
	this->update();   //更新界面
}

//槽函数
void UserWindow::slot_enable_registerAndLogin() {
	int bAok = 0;
	bAok     = ui->input_usrname->text().toStdString().size();
	int bBok = 0;
	bBok     = ui->input_passwd->text().toStdString().size();

	if (bAok > 0 && bBok > 0) {
		ui->btn_register->setEnabled(true);
		ui->btn_login->setEnabled(true);
	} else {
		ui->btn_register->setEnabled(false);
		ui->btn_login->setEnabled(false);
	}
}
void UserWindow::slot_on_login_clicked() {
	MainWindow *p = (MainWindow *)this->parent();   //获取父窗口指针
	//获取界面输入框信息
	std::string nickname = ui->input_usrname->text().toStdString();
	std::string password = ui->input_passwd->text().toStdString();
	//设置用户数据
	p->client->getUser()->setNickname(nickname);
	p->client->getUser()->setPassword(password);

	//提交用户登录Handler
	std::string rawData;
	Handler *task = p->client->dispatch("UserLogin", rawData);
	p->client->submit(task);
}
void UserWindow::slot_on_register_clicked() {
	//按下注册按键
	MainWindow *p = (MainWindow *)this->parent();   //获取父窗口指针
	//获取界面输入框信息
	std::string nickname = ui->input_usrname->text().toStdString();
	std::string password = ui->input_passwd->text().toStdString();
	//设置用户数据
	p->client->getUser()->setNickname(nickname);
	p->client->getUser()->setPassword(password);

	//提交用户注册Handler
	std::string rawData;
	Handler *task = p->client->dispatch("UserRegister", rawData);
	p->client->submit(task);
}
void UserWindow::slot_on_logout_cliecked() {
	MainWindow *p = (MainWindow *)this->parent();   //获取父窗口指针
	//设置登录窗口按钮状态
	ui->btn_register->setEnabled(true);
	ui->btn_login->setEnabled(true);
	ui->btn_logout->setEnabled(false);
	//设置文本框内容
	ui->lb_status->setText("未登录");
	ui->lb_privilege->setText("游客");
	ui->input_usrname->setEnabled(true);
	ui->input_usrname->setText("");
	ui->input_passwd->setEnabled(true);
	ui->input_passwd->setText("");
	this->update();

	//触发登出事件
	std::string rawData;
	Handler *task = p->client->dispatch("UserLogout", rawData);
	p->client->submit(task);
	isLogin = false;

	//发射信号
	emit userUpdata();
}

void UserWindow::paintEvent(QPaintEvent *e) {

	QImage image(QString::fromStdString(m_icon));
	ui->lb_icon->setPixmap(QPixmap::fromImage(image));
	if (isLogin) {
		MainWindow *p = (MainWindow *)this->parent();
		//若已经登录则设置 退出按钮为true
		ui->btn_logout->setEnabled(true);
		ui->input_usrname->setText(QString::fromStdString(p->client->getUser()->getNickname()));
	}
}

// shopcarwindow
ShopCarWindow::ShopCarWindow(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::ShopCarWindow) {
	ui->setupUi(this);
	//设置窗口属性
	this->setWindowFlag(Qt::Dialog);
	//初始化商品展示列表
	ui->table_shopcar->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->table_shopcar->setContextMenuPolicy(Qt::CustomContextMenu);
	QStandardItemModel *model = new QStandardItemModel(0, 5, this);
	model->setHorizontalHeaderItem(0, new QStandardItem(QString("ID")));
	model->setHorizontalHeaderItem(1, new QStandardItem(QString("名称")));
	model->setHorizontalHeaderItem(2, new QStandardItem(QString("价格")));
	model->setHorizontalHeaderItem(3, new QStandardItem(QString("数量")));
	model->setHorizontalHeaderItem(4, new QStandardItem(QString("商家")));
	ui->table_shopcar->setModel(model);
	ui->table_shopcar->setColumnWidth(0, 64);
	ui->table_shopcar->setColumnWidth(1, 130);
	ui->table_shopcar->setColumnWidth(2, 64);
	ui->table_shopcar->setColumnWidth(3, 64);
	ui->table_shopcar->setColumnWidth(4, 104);
	//初始化列表界面右键菜单
	rightMenu         = new QMenu;
	removeFromShopCar = new QAction("从购物车移除");
	rightMenu->addAction(removeFromShopCar);
	connect(ui->btn_buy, SIGNAL(clicked()), this, SLOT(slot_on_buy_clicked()));
	connect(
	    ui->table_shopcar, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slot_show_rightMenu(const QPoint &)));
	connect(removeFromShopCar, SIGNAL(triggered()), this, SLOT(slot_remove_from_shopcar()));
}

ShopCarWindow::~ShopCarWindow() {
	delete ui;
}

void ShopCarWindow::slot_update_table() {
	YURZI_LOG_INFO(YURZI_LOG_NAME("Client")) << "购物车刷新";
	//外界触发的商品显示列表的刷新
	//获取内容模型
	MainWindow *p             = (MainWindow *)this->parent();
	QStandardItemModel *model = (QStandardItemModel *)ui->table_shopcar->model();
	// 1.清空Table控件
	for (int32_t i = model->rowCount() - 1; i >= 0; --i) { model->removeRow(i); }
	// 2.将商品内容加入
	for (auto it : p->client->getShopCar()) {
		uint32_t rowCount = model->rowCount();
		Item::ptr item    = it.second;
		//生成对于的Item
		QStandardItem *i0 = new QStandardItem(QString::number(item->getId()));
		QStandardItem *i1 = new QStandardItem(QString::fromStdString(item->getName()));
		QStandardItem *i2 = new QStandardItem(QString::number(item->getPrice()));
		QStandardItem *i3 = new QStandardItem(QString::number(item->getAmount()));
		QStandardItem *i4 = new QStandardItem(QString::fromStdString(item->getOwner()));
		//将item加入到列中
		model->setItem(rowCount, 0, i0);
		model->setItem(rowCount, 1, i1);
		model->setItem(rowCount, 2, i2);
		model->setItem(rowCount, 3, i3);
		model->setItem(rowCount, 4, i4);
	}
	this->update();   //	重绘窗口
}

void ShopCarWindow::slot_show_rightMenu(const QPoint &) {
	rightMenu->exec(QCursor::pos());
}

void ShopCarWindow::slot_remove_from_shopcar() {
	MainWindow *p = (MainWindow *)this->parent();
	// 获取对于行的商品id
	QStandardItemModel *model = (QStandardItemModel *)ui->table_shopcar->model();
	QModelIndexList selection = ui->table_shopcar->selectionModel()->selectedRows();
	QModelIndex qi;
	int32_t id;
	for (int i = 0; i < selection.count(); ++i) {
		//遍历所有选中的行
		qi = model->index(selection[i].row(), 0);
		id = model->data(qi).toInt();
		//获取到对应的id后从购物车移除
		p->client->removeFromShopCar(id);
	}
	slot_update_table();
	//通知主界面
	p->slot_refresh_cost_label();
}

void ShopCarWindow::slot_on_buy_clicked() {
	//检查用户是否登录
	MainWindow *p = (MainWindow *)this->parent();
	if (p->client->getUser()->getPrivilege() == 0) {
		p->slot_show_error_msg("用户未登录，请登录");
		return;
	}
	//如果登录检查是否有足够的金钱
	double left_money = p->client->getUser()->getMoney() - p->client->getCostMoney();
	if (left_money <= 0) {
		p->slot_show_error_msg("余额不足,请充值");
		return;
	}
	//若都满足则触发购买动作
	p->client->getUser()->setMoney(left_money);   //用户预扣款
	std::string rawData;
	Handler *task = p->client->dispatch("Buy", rawData);
	p->client->submit(task);
}

// AdminWindow
AdminWindow::AdminWindow(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::AdminWindow) {
	ui->setupUi(this);
	//设置窗口属性
	this->setWindowFlag(Qt::Dialog);

	//信号和槽的链接
	connect(ui->btn_user_update, SIGNAL(clicked()), this, SLOT(slot_user_update()));
	connect(ui->btn_user_del, SIGNAL(clicked()), this, SLOT(slot_user_del()));
	connect(ui->btn_item_del, SIGNAL(clicked()), this, SLOT(slot_item_del()));
	connect(ui->btn_item_update, SIGNAL(clicked()), this, SLOT(slot_item_update()));
}
AdminWindow::~AdminWindow() {
	delete ui;
}

void AdminWindow::slot_user_del() {
	MainWindow *p       = (MainWindow *)this->parent();
	std::string usename = ui->ipt_user_nickname->text().toStdString();
	std::string money   = ui->ipt_user_money->text().toStdString();
	if (usename.empty()) {
		QMessageBox::information(this, "错误", "用户名为空");
		return;
	}
	User newUser;
	newUser.setNickname(usename);
	//构建数据包
	DataWarpper datapackage("", "UserDel", DataType::request);
	datapackage.addContent(newUser.toJson());
	std::string rawData = datapackage.toJson();
	Handler *task       = p->client->dispatch("UserDel", rawData);
	p->client->submit(task);
}

void AdminWindow::slot_user_update() {
	MainWindow *p       = (MainWindow *)this->parent();
	std::string usename = ui->ipt_user_nickname->text().toStdString();
	std::string money   = ui->ipt_user_money->text().toStdString();
	if (usename.empty() || money.empty()) {
		QMessageBox::information(this, "错误", "用户名或目标余额为空");
		return;
	}
	User newUser;
	newUser.setNickname(usename);
	newUser.setMoney(ui->ipt_user_money->text().toDouble());
	//构建数据包
	DataWarpper datapackage("", "UserUpdate", DataType::request);
	datapackage.addContent(newUser.toJson());
	std::string rawData = datapackage.toJson();
	Handler *task       = p->client->dispatch("UserUpdate", rawData);
	p->client->submit(task);
}

void AdminWindow::slot_item_del() {
	MainWindow *p = (MainWindow *)this->parent();
	//检查id输入框
	std::string id_input = ui->ipt_item_id->text().toStdString();
	bool Aok             = false;
	int delId            = ui->ipt_item_id->text().toInt(&Aok);
	if (id_input.empty() || !Aok) {
		QMessageBox::information(this, "错误", "ID为空或为非整数");
		return;
	}
	Item delItem;
	delItem.setId(delId);
	//构建数据包
	DataWarpper datapackage("", "ItemDel", DataType::request);
	datapackage.addContent(delItem.toJson());
	std::string rawData = datapackage.toJson();
	Handler* task = p->client->dispatch("ItemDel", rawData);
	p->client->submit(task);
}

void AdminWindow::slot_item_update() {
	MainWindow* p = (MainWindow*) this->parent();
	//检查输入框
	std::string item_id = ui->ipt_item_id->text().toStdString();
	std::string item_name = ui->ipt_item_name->text().toStdString();
	std::string item_price = ui->ipt_item_price->text().toStdString();
	std::string item_amount = ui->ipt_item_amount->text().toStdString();
	std::string item_owner = ui->ipt_item_owner->text().toStdString();
	bool flag = true;
	if (item_name.empty() || item_price.empty() || item_price.empty() || item_amount.empty() || item_owner.empty()) {
		flag = false;
	}
	int id = -1;
	if (flag && (!item_id.empty())) {
		id = ui->ipt_item_id->text().toInt(&flag);
	}
	double price = 0;
	int amount = 0;
	if (flag) {
		bool aok, bok;
		price = ui->ipt_item_price->text().toDouble(&aok);
		amount = ui->ipt_item_amount->text().toInt(&bok);
		flag = aok && bok;
	}

	if (!flag) {
		QMessageBox::information(this, "错误", "请将所有输入框填上合适的值（id选填,若填写请为整数）");
		return;
	}
	//构建数据包
	Item newItem(id, item_name, price, amount, item_owner, "");
	DataWarpper datapackage("", "ItemUpdate", DataType::request);
	datapackage.addContent(newItem.toJson());
	std::string rawData = datapackage.toJson();
	Handler * task = p->client->dispatch("ItemUpdate", rawData);
	p->client->submit(task);
}
}   // namespace yolspc