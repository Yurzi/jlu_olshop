#include "log.h"

#include "utils.h"
#include <ctime>
#include <iostream>

int main(int argc, char const *argv[]) {
	yurzilog::Logger::ptr logger(std::make_shared<yurzilog::Logger>());
	logger->addAppender(std::make_shared<yurzilog::StdoutLogAppender>());

	yurzilog::FileLogAppender::ptr file_appender(std::make_shared<yurzilog::FileLogAppender>("./yurzilog.yurzilog"));
	logger->addAppender(file_appender);

	yurzilog::LogFormatter::ptr fmt(std::make_shared<yurzilog::LogFormatter>("%d%T%p%T%m%n"));
	file_appender->setFormatter(fmt);
	file_appender->setLevel(yurzilog::LogLevel::ERROR);

	// yurzilog::LogEvent::ptr event(std::make_shared<yurzilog::LogEvent>(__FILE__, __LINE__, 0, yolspc::GetThreadId(),
	// yolspc::GetFiberId(), time(0))); event->getSS() << "我真的哭死";

	// logger->yurzilog(yurzilog::LogLevel::DEBUG, event);

	YURZI_LOG_INFO(logger) << "让我康康";
	YURZI_LOG_INFO(logger) << "再看一眼";
	YURZI_LOG_ERROR(logger) << "出大问题";

	YURZI_LOG_FMT_FATAL(logger, "出大问题 %s", "让我康康");

	auto l = yurzilog::LoggerMgr::GetInstance()->getLogger("xx");
	YURZI_LOG_INFO(l) << "??????";
	return 0;
}