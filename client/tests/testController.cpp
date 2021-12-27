#include "client.h"
#include <QApplication>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	//初始化控制器
	std::shared_ptr<yolspc::ClientController> cl(new yolspc::ClientController);
	cl->init();
	return a.exec();
}