#include "socket.h"
#include <QApplication>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	yolspc::SocketController::ptr qwq(new yolspc::SocketController(11451));
	// yolspc::Socketer::ptr test(new yolspc::Socketer("127.0.0.1", 11451, qwq));

	std::cout << "qwq?" << std::endl;

	// test->connectToHost();
	//  std::string content;
	//  std::cin >> content;
	//  std::cout << content << std ::endl;

	// test->write(qwq);
	return QApplication::exec();
}