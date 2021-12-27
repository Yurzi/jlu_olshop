#ifndef __YOLSPC_DATAWARPPER_H__
#define __YOLSPC_DATAWARPPER_H__

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <chrono>
#include <memory>

namespace yolspc {

class DataType {
public:
	inline static const std::string request  = "request";
	inline static const std::string response = "response";
	inline static const std::string heart    = "heart";
};

class DataWarpper {
public:
	typedef std::shared_ptr<DataWarpper> ptr;

	//构造函数
	DataWarpper(const std::string &input_session, const std::string input_handler, const std::string input_type) {
		session_id = input_session;
		//获取当前时间戳
		auto timepoint = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
		auto time      = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint.time_since_epoch());
		timestamp      = time.count();
		//设置HandlerName
		handlerName = input_handler;
		type        = input_type;
		status      = 200;
	}

	DataWarpper(const std::string &input_session,
	            const std::string input_handler,
	            const std::string input_type,
	            uint32_t input_status) {
		session_id = input_session;
		//获取当前时间戳
		auto timepoint = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
		auto time      = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint.time_since_epoch());
		timestamp      = time.count();
		//设置HandlerName
		handlerName = input_handler;
		type        = input_type;
		status      = input_status;
	}

	//添加contenets内容
	void addContent(std::string content) { contents.append(QString::fromStdString(content)); }

	//输出Json字符串
	const std::string toJson() {
		QJsonObject root_obj;
		root_obj.insert("session", session_id.c_str());
		root_obj.insert("time", QJsonValue::fromVariant(QVariant(timestamp)));
		root_obj.insert("handler", handlerName.c_str());
		root_obj.insert("type", type.c_str());
		root_obj.insert("status", QJsonValue::fromVariant(QVariant(status)));
		root_obj.insert("contents", contents);
		QJsonDocument doc(root_obj);
		return doc.toJson().toStdString();
	}
	//输出JsonObjet
	const QJsonObject toJsonObject() {
		QJsonObject root_obj;
		root_obj.insert("session", session_id.c_str());
		root_obj.insert("time", QJsonValue::fromVariant(QVariant(timestamp)));
		root_obj.insert("handler", handlerName.c_str());
		root_obj.insert("type", type.c_str());
		root_obj.insert("status", QJsonValue::fromVariant(QVariant(status)));
		root_obj.insert("contents", contents);
		return root_obj;
	}

public:
	std::string session_id;
	uint64_t timestamp;
	std::string handlerName;
	std::string type;
	uint32_t status;
	QJsonArray contents;
};
}   // namespace yolspc

#endif   //__YOLSPC_DATAWARPPER_H__