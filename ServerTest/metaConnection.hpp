#pragma once
#include "WebSocketCommon.hpp"
/*
元连接模块，代表一个连接，里面包含了单独连接所有的内容，每一个活动连接均会开启一个线程进行数据处理与通讯
目前的设置就是，一旦设置好了method那么该对应连接的所有通讯都会以设置好的策略进行
*/
extern class ControlModule;
class metaConnection {
public:
	enum class connectionStatus {
		OPEN,
		CLOSE,
	};
	enum class Method {
		UNDEFINED,
		IMG_RETRANSMISSION,//目前仅支持此方法进行
	};
	metaConnection() = delete;
	/*
	MSG_MAX: 缓存中所保存的最大数据数量，超过该数量则会清空缓存
	*/
	metaConnection(server& ser, websocketpp::connection_hdl hdl, const int& MSG_MAX = 1000, Method a = Method::UNDEFINED) :m_hdl(hdl), m_status(connectionStatus::OPEN), m_method(a), m_endpoint(ser), FRAMEPOOLMAXSIZE(MSG_MAX) {
		m_alias = "undefined";//昵称（别名的未定义状态）
		m_thrEndLabel = false;
		start();
	};
	~metaConnection() {
		//线程join之后不能在此join此处是因为其他地方可能调用stop
		if (m_thread.joinable()) {
			stop();
		}
	};
	void start(){
		m_thread = std::thread([&]() {
			while (true) 
			{
				if (m_thrEndLabel)break;
				std::string sinFrame;
				if (!getFrame(sinFrame)) {
					std::this_thread::yield();
					continue;
				}
				{
					std::lock_guard lk(m_lock);
					switch (m_method)
					{
					case Method::IMG_RETRANSMISSION:
						for (auto i : m_toMetaCon) 
						{
							i->SentBinary(sinFrame);
						}
						break;
					default:
						break;
					}
				}
				std::this_thread::yield();
			}
		}
		);
	}
	void stop() {
		m_thrEndLabel = true;
		m_thread.join();
	}
	inline void SetMethod(const std::string& method) {
		if (method == "ImgRetransmission") {
			m_method = Method::IMG_RETRANSMISSION;
		}
	}
	inline void setName(const std::string& alias) {
		std::lock_guard lk(m_lock);
		m_alias = alias;
	}
	inline bool getName(std::string& name) {
		std::lock_guard lk(m_lock);
		if (m_alias != "undefined") {
			name = m_alias;
			return true;
		}
		return false;
	}
	void SendTo(const std::string& msg) {
		if (m_status != connectionStatus::OPEN)return;
		websocketpp::lib::error_code ec;
		m_endpoint.send(m_hdl, msg, websocketpp::frame::opcode::text, ec);
		if (ec) {
			std::cout << "> Error send: " << msg
				<< ec.message() << std::endl;
			m_status = connectionStatus::CLOSE;
		}
	}
	void SentBinary(const std::string& msg) {
		if (m_status != connectionStatus::OPEN)return;
        websocketpp::lib::error_code ec;
		std::vector<uint8_t> biMsg(msg.begin(), msg.end());
		m_endpoint.send(m_hdl, biMsg.data(), biMsg.size(), websocketpp::frame::opcode::BINARY,ec);
        if (ec) {
            std::cout << "> Error sending binary: " << ec.message() << std::endl;
			m_status = connectionStatus::CLOSE;
            return;
        }
	}
	void feedFrame(const std::string& frame) {
		std::lock_guard lk(m_lock);
		if (m_status == connectionStatus::OPEN) {
			if (m_method == Method::UNDEFINED) {
				SendTo("Please define a method yet...");
				return;
			}
			if (m_framePool.size() >= FRAMEPOOLMAXSIZE) {
				//超过全丢
				m_framePool.clear();
				SendTo("Warn: framePool overwhelm, clear all frame..");
			}
			m_framePool.push_back(frame);
			return;
		}
		std::cout << "warn Name: " << m_alias << " is not open----" << int(m_status);
	}
	inline bool getFrame(std::string& frame, bool pop = true) {
		std::lock_guard lk(m_lock);
		if (m_framePool.empty()) {
			return false;
		}
		frame = std::move(m_framePool.front());
		if (pop) {
			m_framePool.pop_front();
		}
		return true;
	}
	inline void setStatus(connectionStatus status) {
		m_status = status;
	}
	inline connectionStatus getStatus() {
		return m_status;
	}
	inline websocketpp::connection_hdl getHdl() {
		return m_hdl;
	}

	inline void addMetaCon(websocketpp::lib::shared_ptr<metaConnection> a) {
		std::lock_guard lk(m_lock);
		m_toMetaCon.push_back(a);
	}
	inline std::list<websocketpp::lib::shared_ptr<metaConnection>> getMetaCon() {
		return m_toMetaCon;
	}
	inline void replaceMetaCon(const std::string& alias, websocketpp::lib::shared_ptr<metaConnection>ptr) {
		std::lock_guard lk(m_lock);
		for (auto i = m_toMetaCon.begin(); i != m_toMetaCon.end();++i) {
			std::string name;
			if ((*i)->getName(name)) {
				if (!name.compare(alias)) {
					//查找到对象 ,替换
					*i = ptr;
					break;
				}
			}
		}
	}
private:
	server& m_endpoint;
	const int FRAMEPOOLMAXSIZE;
	std::mutex m_lock;
	connectionStatus m_status;
	std::thread m_thread;
	bool m_thrEndLabel;
	std::list<std::string>m_framePool;
	std::string m_alias;//初始为undefined,代表自己的名字
	std::list<websocketpp::lib::shared_ptr<metaConnection>>m_toMetaCon;//需要进行通讯的连接
	//std::vector<std::string> m_toAlias;
	Method m_method;
	websocketpp::connection_hdl m_hdl;//代表自身的连接
};
