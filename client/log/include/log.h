#ifndef __LOG_H__
#define __LOG_H__

#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdint.h>
#include <string>
#include <thread>
#include <vector>

//宏定义 方便日志调用

//流式输出
#define YURZI_LOG_LEVEL(logger, level)                                                      \
    if (logger->getLevel() <= level)                                                        \
    yurzilog::LogEventWarpper(std::make_shared<yurzilog::LogEvent>(logger,                  \
                                                                   level,                   \
                                                                   __FILE__,                \
                                                                   __LINE__,                \
                                                                   0,                       \
                                                                   yurzilog::GetThreadId(), \
                                                                   yurzilog::GetFiberId(),  \
                                                                   time(0)))                \
        .getSS()

#define YURZI_LOG_DEBUG(logger) YURZI_LOG_LEVEL(logger, yurzilog::LogLevel::DEBUG)
#define YURZI_LOG_INFO(logger) YURZI_LOG_LEVEL(logger, yurzilog::LogLevel::INFO)
#define YURZI_LOG_WARN(logger) YURZI_LOG_LEVEL(logger, yurzilog::LogLevel::WARN)
#define YURZI_LOG_ERROR(logger) YURZI_LOG_LEVEL(logger, yurzilog::LogLevel::ERROR)
#define YURZI_LOG_FATAL(logger) YURZI_LOG_LEVEL(logger, yurzilog::LogLevel::FATAL)

//格式化式输出
#define YURZI_LOG_FMT_LEVEL(logger, level, fmt, ...)                                        \
    if (logger->getLevel() <= level)                                                        \
    yurzilog::LogEventWarpper(std::make_shared<yurzilog::LogEvent>(logger,                  \
                                                                   level,                   \
                                                                   __FILE__,                \
                                                                   __LINE__,                \
                                                                   0,                       \
                                                                   yurzilog::GetThreadId(), \
                                                                   yurzilog::GetFiberId(),  \
                                                                   time(0)))                \
        .getEvent()                                                                         \
        ->format(fmt, ##__VA_ARGS__)

#define YURZI_LOG_FMT_DEBUG(logger, fmt, ...) \
    YURZI_LOG_FMT_LEVEL(logger, yurzilog::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define YURZI_LOG_FMT_INFO(logger, fmt, ...) \
    YURZI_LOG_FMT_LEVEL(logger, yurzilog::LogLevel::INFO, fmt, ##__VA_ARGS__)
#define YURZI_LOG_FMT_WARN(logger, fmt, ...) \
    YURZI_LOG_FMT_LEVEL(logger, yurzilog::LogLevel::WARN, fmt, ##__VA_ARGS__)
#define YURZI_LOG_FMT_ERROR(logger, fmt, ...) \
    YURZI_LOG_FMT_LEVEL(logger, yurzilog::LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define YURZI_LOG_FMT_FATAL(logger, fmt, ...) \
    YURZI_LOG_FMT_LEVEL(logger, yurzilog::LogLevel::FATAL, fmt, ##__VA_ARGS__)

//日志器管理
#define YURZI_LOG_ROOT() yurzilog::LoggerMgr::GetInstance()->getRoot()
#define YURZI_LOG_NAME(name) yurzilog::LoggerMgr::GetInstance()->getLogger(name)

namespace yurzilog {

//单例模板
template <class T, class X = void, int N = 0> class Singleton {
public:
	static T *GetInstance() {
		static T v;
		return &v;
	}
};
//单例指针模板
template <class T, class X = void, int N = 0> class SingletonPtr {
	static std::shared_ptr<T> GetInstance() {
		static std::shared_ptr<T> v(new T);
		return v;
	}
};

class Logger;

//日志等级
class LogLevel {
public:
	enum Level { UNKNOW = 0, DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4, FATAL = 5 };

	static const char *ToString(LogLevel::Level level);
	static LogLevel::Level FromString(const std::string &str);
};

//日志事件
class LogEvent {
public:
	typedef std::shared_ptr<LogEvent> ptr;
	LogEvent(std::shared_ptr<Logger> logger,
	         LogLevel::Level level,
	         const char *file,
	         int32_t line,
	         uint32_t elapse,
	         uint32_t thread_id,
	         uint32_t fiber_id,
	         uint64_t time);

	const char *getFile() const { return m_file; }
	int32_t getLine() const { return m_line; }
	uint32_t getElapse() const { return m_elapse; }
	uint32_t getThreadId() const { return m_threadId; }
	uint32_t getFiberId() const { return m_fiberId; }
	uint64_t getTime() const { return m_time; }
	std::shared_ptr<Logger> getLogger() { return m_logger; }
	LogLevel::Level getLevel() { return m_level; }
	const std::string getContent() const { return m_ss.str(); }
	std::stringstream &getSS() { return m_ss; }

	void format(const char *fmt, ...);
	void format(const char *fmt, va_list al);

private:
	const char *m_file  = nullptr;      //文件名
	int32_t m_line      = 0;            //行号
	uint32_t m_elapse   = 0;            //程序启动开始到现在的毫秒
	uint32_t m_threadId = 0;            //线程id
	uint32_t m_fiberId  = 0;            //协程id
	uint64_t m_time;                    //时间戳
	std::stringstream m_ss;             //日志内容
	std::shared_ptr<Logger> m_logger;   //目标日志器
	LogLevel::Level m_level;            //日志等级
};

//日志包装器
class LogEventWarpper {
public:
	typedef std::shared_ptr<LogEventWarpper> ptr;

	LogEventWarpper(LogEvent::ptr event);
	~LogEventWarpper();

	LogEvent::ptr getEvent() { return m_event; }

	std::stringstream &getSS();

private:
	LogEvent::ptr m_event;
};

//日志格式化器
class LogFormatter {
public:
	typedef std::shared_ptr<LogFormatter> ptr;
	LogFormatter(const std::string &pattern);

	//格式化输出
	std::string format(std::shared_ptr<Logger> logger,
	                   LogLevel::Level level,
	                   LogEvent::ptr event);   //将日记输出格式化

public:
	//格式对象
	class FormatItem {
	public:
		typedef std::shared_ptr<FormatItem> ptr;

		virtual ~FormatItem() {}
		virtual void format(std::shared_ptr<Logger> logger,
		                    std::ostream &os,
		                    LogLevel::Level level,
		                    LogEvent::ptr event) = 0;
	};

	/**
	 * %m 消息体
	 * %p 日志level
	 * %r 累计毫秒数
	 * %c 日志名称
	 * %t 线程id
	 * %n 回车
	 * %d 时间
	 * %f 文件名
	 * %l 行号
	 * %T 制表符
	 * %F 协程Id
	 */
	void init();

private:
	std::string m_pattern;
	std::vector<FormatItem::ptr> m_items;   //存放可行的日志格式对象
};

//日志输出地
class LogAppender {
public:
	typedef std::shared_ptr<LogAppender> ptr;

	//用于不同的LogAppender析构
	virtual ~LogAppender() {};

	virtual void
	log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
	void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
	LogFormatter::ptr getFormatter() const { return m_formatter; }
	void setLevel(LogLevel::Level level) { m_level = level; }
	LogLevel::Level getLevel() const { return m_level; }

protected:
	LogLevel::Level m_level = LogLevel::DEBUG;
	LogFormatter::ptr m_formatter;
};

//日志器
class Logger : public std::enable_shared_from_this<Logger> {
public:
	typedef std::shared_ptr<Logger> ptr;

	Logger(const std::string &name = "root");
	void log(LogLevel::Level level, const LogEvent::ptr event);

	//输出各种级别的日志
	void debug(LogEvent::ptr event);
	void info(LogEvent::ptr event);
	void warn(LogEvent::ptr event);
	void error(LogEvent::ptr event);
	void fatal(LogEvent::ptr event);

	void addAppender(LogAppender::ptr appender);
	void delAppender(LogAppender::ptr appender);

	LogLevel::Level getLevel() const { return m_level; }
	void setLevel(LogLevel::Level val) { m_level = val; }
	void setFormatter(LogFormatter::ptr fmt) { m_formatter = fmt; }
	LogFormatter::ptr getFormatter() const { return m_formatter; }

	const std::string &getName() const { return m_name; }

private:
	std::string m_name;        //日志器名称，用于区分
	LogLevel::Level m_level;   //只有满足日志级别的日志才会被输出
	std::list<LogAppender::ptr> m_appenders;
	LogFormatter::ptr m_formatter;   //默认的formatter
};

//日志输出地：标准控制台输出
class StdoutLogAppender : public LogAppender {
public:
	typedef std::shared_ptr<StdoutLogAppender> ptr;
	void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

private:
	std::mutex lock;
};

//日志输出地：标准文件输出
class FileLogAppender : public LogAppender {
public:
	typedef std::shared_ptr<FileLogAppender> ptr;

	FileLogAppender(const std::string &filename);
	void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

	//重新打开文件，打开成功返回true
	bool reopen();

private:
	std::string m_filename;
	std::ofstream m_filestream;
	std::mutex lock;
};

//日志器管理器

class LoggerManager {
public:
	LoggerManager();
	Logger::ptr getLogger(const std::string& name = "root");
	bool addLogger(Logger::ptr logger);

	Logger::ptr getRoot() const {return m_root;}

	void init();
private:
	std::map<std::string, Logger::ptr> m_loggers;
	Logger::ptr m_root;	//主logger
};

//日志器管理器单例模式
typedef Singleton<LoggerManager> LoggerMgr;


//获取线程id
uint32_t GetThreadId();

//获取协程id
uint32_t GetFiberId();

}   // namespace yurzilog
#endif //__LOG_H__
