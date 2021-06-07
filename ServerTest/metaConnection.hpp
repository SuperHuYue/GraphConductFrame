#pragma once
#include "WebSocketCommon.hpp"
/*
元连接模块，代表一个连接，里面包含了单独连接所有的内容，每一个活动连接均会开启一个线程进行数据处理与通讯
目前的设置就是，一旦设置好了method那么该对应连接的所有通讯都会以设置好的策略进行
*/

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
	metaConnection(server& ser, websocketpp::connection_hdl hdl, const int& MSG_MAX = 1000, Method a = Method::UNDEFINED) :m_hdl(hdl), m_status(connectionStatus::OPEN), m_method(a), m_endPoint(ser), FRAMEPOOLMAXSIZE(MSG_MAX) {
		m_alias = "undefined";//昵称（别名的未定义状态）
		m_thrEndLabel = false;
		start();
	};
	~metaConnection() {
		stop();
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
				switch (m_method)
				{
				case Method::IMG_RETRANSMISSION:
					for (auto i : m_toMetaCon) {
						if (!i.expired()) {
							auto tar = i.lock();
							//tar->SendTo()
							tar->SentBinary(sinFrame);
						}
					}
					break;
				default:
					break;
				}

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
		m_endPoint.send(m_hdl, msg, websocketpp::frame::opcode::text, ec);
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
        m_endPoint.send(m_hdl, biMsg.data(), biMsg.size(), websocketpp::frame::opcode::BINARY,ec);
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


	std::mutex m_lock;
	connectionStatus m_status;
	std::list<websocketpp::lib::weak_ptr<metaConnection>>m_toMetaCon;//需要进行通讯的连接
	Method m_method;
	websocketpp::connection_hdl m_hdl;//代表自身的连接
private:
	std::thread m_thread;
	bool m_thrEndLabel;
	server& m_endPoint;
	const int FRAMEPOOLMAXSIZE;
	std::list<std::string>m_framePool;
	std::string m_alias;//初始为undefined
};
