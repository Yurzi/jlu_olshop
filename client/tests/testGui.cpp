#include "qtgui.h"
#include <QApplication>

int main(int argc, char *argv[]) {
	Q_INIT_RESOURCE(assets);
	QApplication a(argc, argv);
	yolspc::MainWindow mainwindow;
	mainwindow.showNormal();
	// Q_CLEANUP_RESOURCE(assets);
	return QApplication::exec();
}