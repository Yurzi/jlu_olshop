#include "log.h"

#include "extraio.h"
#include <ctime>
#include <functional>
#include <iostream>
#include <stdarg.h>

namespace yurzilog {

// LogLevel

const char *LogLevel::ToString(LogLevel::Level level) {
    switch (level) {
#define XX(name) \
    case LogLevel::name: return #name; break;

        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
    default: return "UNKNOW";
    }
    return "UNKNOW";
}

LogLevel::Level LogLevel::FromString(const std::string &str) {
#define XX(level, v) \
    if (str == #v) { return LogLevel::level; }

    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
    return LogLevel::UNKNOW;
#undef XX
}

// Logger

Logger::Logger(const std::string &name)
    : m_name(name)
    , m_level(LogLevel::DEBUG) {
    m_formatter = std::make_shared<LogFormatter>(
                      "[%d{%Y-%m-%d %H:%M:%S}][%p][%c][Thread-%t][%f:%l]:%m%n");
}

void Logger::log(LogLevel::Level level, const LogEvent::ptr event) {
    if (level >= m_level) {
        auto self = shared_from_this();
        for (auto &i : m_appenders) { i->log(self, level, event); }
    }
}

//输出各种级别的日志
void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}
void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}
void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}
void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}
void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}

void Logger::addAppender(LogAppender::ptr appender) {
    if (!appender->getFormatter()) { appender->setFormatter(m_formatter); }
    m_appenders.push_back(appender);
}
void Logger::delAppender(LogAppender::ptr appender) {
    //遍历查找链表实现删除对应的appender
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

// LogAppender

FileLogAppender::FileLogAppender(const std::string &filename)
    : m_filename(filename) {
    reopen();
}

void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {

    if (level >= m_level) {
        std::unique_lock<std::mutex> locker(lock);   //进行一个锁的上
        if (!m_filestream.is_open()) { reopen(); }
        m_filestream << m_formatter->format(logger, level, event);
        m_filestream.close();
    }
}

bool FileLogAppender::reopen() {
    if (m_filestream) { m_filestream.close(); }
    m_filestream.open(m_filename, std::ios::app);
    return !!m_filestream;
}

void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    std::unique_lock<std::mutex> locker(lock);   //进行一个锁的上
    if (level >= m_level) { std::cout << m_formatter->format(logger, level, event); }
}

// LogFormatter

LogFormatter::LogFormatter(const std::string &pattern)
    : m_pattern(pattern) {
    //初始化
    init();
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger,
                                 LogLevel::Level level,
                                 LogEvent::ptr event) {
    std::stringstream ss;
    for (auto &i : m_items) { i->format(logger, ss, level, event); }
    return ss.str();
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << logger->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
        : m_format(format) {
        if (m_format.empty()) { m_format = "%Y-%m-%d %H:%M:%S"; }
    }

    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        //获取系统时间
        time_t lt = event->getTime();
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), localtime(&lt));
        os << buf;
    }

private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << std::endl;
    }
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << '\t';
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string &str = "")
        : m_string(str) {}
    void format(std::shared_ptr<Logger> logger,
                std::ostream &os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << m_string;
    }

private:
    std::string m_string;
};

//%xxx %xxx{xxx} %% %str{fmt}
void LogFormatter::init() {
    //三元组的格式 str format type #存在的格式的数组及顺序
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;   //代表非%内的字符串，及非格式内容

    //将m_pattern内的格式信息进行解析,遍历pattern  其逻辑类似于有限状态机
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            //若未遇到% 则patter中的内容为 源字符串不解析。
            nstr.append(1, m_pattern[i]);
            continue;   //继续遍历
        }
        //若i+1是%则证明真的是%
        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        //若是%则开始解析格式
        size_t n         = i + 1;   //非%的第一个字符
        int fmt_status   = 0;       //当前解析器的状态
        size_t fmt_begin = 0;       //记录解析器解析{}的初始状态

        std::string str;   //存放格式信息
        std::string fmt;   //存放{}内的内容
        while (n < m_pattern.size()) {
            //取出一个格式信息
            if (!fmt_status &&
                    (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '{') {
                    str        = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1;   //处于解析格式状态
                    ++n;
                    fmt_begin = n;
                    continue;
                }
            }
            if (fmt_status == 1) {
                if (m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin, n - fmt_begin);   //取出括号内内容
                    fmt_status = 0;
                    ++n;
                    break;   //结束一个{}格式的解析。
                }
            }
            ++n;   //若为常规字符
            //边界特判
            if (n == m_pattern.size()) {
                if (str.empty()) { str = m_pattern.substr(i + 1); }
            }
        }

        //解析完一个格式之后
        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            // str = m_pattern.substr(i + 1, n - i - 1);
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;   //用于抵消i++带来的偏移。
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error" << m_pattern << " - " << m_pattern.substr(i)
                      << std::endl;
            vec.push_back(std::make_tuple(" << pattern_error>>", fmt, 0));
        }
    }
    //处理结尾
    if (!nstr.empty()) { vec.push_back(std::make_tuple(nstr, std::string(), 0)); }

    static std::map<std::string, std::function<FormatItem::ptr(const std::string &)>>
    s_format_items = {
#define XX(str, C)                                                               \
    {                                                                            \
#str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); } \
    }

        XX(m, MessageFormatItem),    // m:消息
        XX(p, LevelFormatItem),      // p:日志级别
        XX(r, ElapseFormatItem),     // r:累计毫秒数
        XX(c, NameFormatItem),       // c:日志名称
        XX(t, ThreadIdFormatItem),   // t:线程id
        XX(n, NewLineFormatItem),    // n:换行
        XX(d, DateTimeFormatItem),   // d:时间
        XX(f, FilenameFormatItem),   // f:文件名
        XX(l, LineFormatItem),       // l:行号
        XX(T, TabFormatItem),        // T:制表符
        XX(F, FiberIdFormatItem),    // F:协程id

#undef XX
    };
    for (auto &i : vec) {
        if (std::get<2>(i) == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(
                                      new StringFormatItem(" << error_format. % " + std::get<0>(i) + " >> ")));
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
        // std::cout << std::get<0>(i) << " - " << std::get<1>(i) << " - " << std::get<2>(i) <<
        // std::endl;
    }
}

// LogEvent

LogEvent::LogEvent(std::shared_ptr<Logger> logger,
                   LogLevel::Level level,
                   const char *file,
                   int32_t line,
                   uint32_t elapse,
                   uint32_t thread_id,
                   uint32_t fiber_id,
                   uint64_t time)
    : m_logger(logger)
    , m_level(level)
    , m_file(file)
    , m_line(line)
    , m_elapse(elapse)
    , m_threadId(thread_id)
    , m_fiberId(fiber_id)
    , m_time(time) {}

void LogEvent::format(const char *fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char *fmt, va_list al) {
    char *buf = nullptr;
    int len   = vasprintf(&buf, fmt, al);
    if (len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

// LogEventWarpper

LogEventWarpper::LogEventWarpper(LogEvent::ptr event)
    : m_event(event) {}

LogEventWarpper::~LogEventWarpper() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

std::stringstream &LogEventWarpper::getSS() {
    return m_event->getSS();
}

// LoggerManager

LoggerManager::LoggerManager() {
    //添加默认logger
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
}

Logger::ptr LoggerManager::getLogger(const std::string &name) {
    auto it = m_loggers.find(name);
    return it == m_loggers.end() ? m_root : it->second;   //返回指定名称的日志器
}

bool LoggerManager::addLogger(Logger::ptr logger) {
    auto it = m_loggers.find(logger->getName());
    if (it != m_loggers.end()) {
        YURZI_LOG_ERROR(m_root) << "无法添加Name为" << logger->getName() << "的日志器";
        return false;
    }
    m_loggers[logger->getName()] = logger;  //插入对应名称的日志器
    YURZI_LOG_INFO(m_root) << "成功插入Name为" << logger->getName() << "的日志器";
    return true;
}

uint32_t GetThreadId() {
    std::stringstream oss;
    oss << std::this_thread::get_id();
    std::string stld = oss.str();
    return std::stoull(stld);
}

uint32_t GetFiberId() {
    return 0;
}

}   // namespace yurzilog
