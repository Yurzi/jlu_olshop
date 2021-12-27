#ifndef __YOLSPC_DATATYPE_H__
#define __YOLSPC_DATATYPE_H__

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <memory>
#include <string>

namespace yolspc {

//用户对象
class User {
public:
	typedef std::shared_ptr<User> ptr;

	//构造函数
	User() {
		m_id        = 0;
		m_nickname  = "";
		m_password  = "";
		m_privilege = 0;
		m_money     = 0;
		m_icon      = "";
	}
	User(std::string nickname, std::string password = "", uint8_t privilege = 0, double money = 0, std::string icon = "")
		: m_nickname(nickname)
		, m_password(password)
		, m_privilege(privilege)
		, m_money(money)
		, m_icon(icon) {}

	User(uint32_t id,
	     std::string nickname = "",
	     std::string password = "",
	     uint8_t privilege    = 0,
	     double money         = 0,
	     std::string icon     = "")
		: m_id(id)
		, m_nickname(nickname)
		, m_password(password)
		, m_privilege(privilege)
		, m_money(money)
		, m_icon(icon) {}

	// Getter
	uint32_t getId() const { return m_id; }
	const std::string& getNickname() const { return m_nickname; }
	const std::string& getPassword() const { return m_password; }
	uint8_t getPrivilege() const { return m_privilege; }
	double getMoney() const { return m_money; }
	const std::string& getIcon() const { return m_icon; }
	std::string getPrivilegeString() {
		std::string privilege;
		switch (m_privilege) {
		case 0: privilege = "游客"; break;
		case 1: privilege = "用户"; break;
		case 2: privilege = "卖家"; break;
		case 3: privilege = "管理员"; break;
		default: privilege = "什么都不是，爬！";
		}
		return privilege;
	}

	// Setter
	void setId(const uint32_t id) { m_id = id; };
	void setNickname(const std::string nickname) { m_nickname = nickname; }
	void setPassword(const std::string password) { m_password = password; }
	void setPrivilege(const uint8_t privilege) { m_privilege = privilege; }
	void setMoney(const double money) { m_money = money; }
	void setIcon(const std::string icon) { m_icon = icon; }
	void resetDefault() {
		//将用户对象设为默认
		m_id        = 0;
		m_nickname  = "";
		m_password  = "";
		m_privilege = 0;
		m_money     = 0;
		m_icon      = "";
	}

	//转化为Json
	const std::string toJson() {
		QJsonObject root_obj;
		root_obj.insert("id", QJsonValue::fromVariant(QVariant(m_id)));
		root_obj.insert("nickname", QString::fromStdString(m_nickname));
		root_obj.insert("password", QString::fromStdString(m_password));
		root_obj.insert("privilege", m_privilege);
		root_obj.insert("money", m_money);
		root_obj.insert("icon", QString::fromStdString(m_icon));

		QJsonDocument doc(root_obj);
		return doc.toJson().toStdString();
	}

	void fromJson(const std::string& jsonString) {
		QJsonDocument document = QJsonDocument::fromJson(QByteArray::fromStdString(jsonString));
		QJsonObject root_obj   = document.object();
		m_id                   = root_obj.value("id").toVariant().toUInt();
		m_nickname             = root_obj.value("nickname").toVariant().toString().toStdString();
		m_password             = root_obj.value("password").toVariant().toString().toStdString();
		m_privilege            = root_obj.value("privilege").toVariant().toChar().toLatin1();
		m_money                = root_obj.value("money").toVariant().toDouble();
		m_icon                 = root_obj.value("icon").toVariant().toString().toStdString();
	}

private:
	uint32_t m_id;            //用户id
	std::string m_nickname;   //用户名
	std::string m_password;   //密码
	uint8_t m_privilege;
	double m_money;       //余额
	std::string m_icon;   //头像路径

public:
	class Privilege {
	public:
		inline static uint8_t vistor   = 0;
		inline static uint8_t customer = 1;
		inline static uint8_t shopper  = 2;
		inline static uint8_t root     = 3;
	};
};

//商品对象
class Item {
public:
	typedef std::shared_ptr<Item> ptr;
	Item() {}

	Item(const Item::ptr& oldItem) {
		m_id     = oldItem->getId();
		m_name   = oldItem->getName();
		m_pic    = oldItem->getPic();
		m_price  = oldItem->getPrice();
		m_amount = oldItem->getAmount();
		m_owner  = oldItem->getOwner();
	}

	// 构造函数
	Item(const int32_t id,
	     const std::string& name,
	     const double& price,
	     const int32_t amount,
	     const std::string& owner,
	     const std::string& pic = nullptr)
		: m_id(id)
		, m_name(name)
		, m_price(price)
		, m_amount(amount)
		, m_owner(owner)
		, m_pic(pic) {}

	// Getter
	const int32_t& getId() const { return m_id; }
	const std::string& getName() const { return m_name; }
	const std::string& getPic() const { return m_pic; }
	const double& getPrice() const { return m_price; }
	const int32_t& getAmount() const { return m_amount; }
	const std::string& getOwner() const { return m_owner; }

	// Setter
	void setId(const int32_t& id) { m_id = id; }
	void setName(const std::string& name) { m_name = name; }
	void setPic(const std::string& pic) { m_pic = pic; }
	void setPrice(const double& price) { m_price = price; }
	void setAmount(const int32_t& amount) { m_amount = amount; }
	void setOwner(const std::string& owner) { m_owner = owner; }

	//转化为Json
	const std::string toJson() {
		QJsonObject root_obj;
		root_obj.insert("id", m_id);
		root_obj.insert("name", QString::fromStdString(m_name));
		root_obj.insert("pic", QString::fromStdString(m_pic));
		root_obj.insert("price", m_price);
		root_obj.insert("amount", m_amount);
		root_obj.insert("owner", QString::fromStdString(m_owner));

		QJsonDocument doc(root_obj);
		return doc.toJson().toStdString();
	}

	void fromJson(const std::string& jsonString) {
		QJsonDocument document = QJsonDocument::fromJson(QByteArray::fromStdString(jsonString));
		QJsonObject root_obj   = document.object();
		m_id                   = root_obj.value("id").toVariant().toInt();
		m_name                 = root_obj.value("name").toVariant().toString().toStdString();
		m_pic                  = root_obj.value("pic").toVariant().toString().toStdString();
		m_price                = root_obj.value("price").toVariant().toDouble();
		m_amount               = root_obj.value("amount").toVariant().toInt();
		m_owner                = root_obj.value("owner").toVariant().toString().toStdString();
	}

private:
	int32_t m_id        = -1;
	std::string m_name  = "";
	std::string m_pic   = "";
	double m_price = 0;
	int32_t m_amount = 0;
	std::string m_owner = "";
};

}   // namespace yolspc

#endif