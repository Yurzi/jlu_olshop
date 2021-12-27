#include "client.h"
#include "log.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	Q_INIT_RESOURCE(assets);   //载入Qt资源
	//初始化主日志器,为其添加文件输出
	YURZI_LOG_ROOT()->addAppender(std::make_shared<yurzilog::FileLogAppender>("log.log"));
	//初始化控制器
	std::shared_ptr<yolspc::ClientController> cl(new yolspc::ClientController);
	cl->init();
	return a.exec();
}