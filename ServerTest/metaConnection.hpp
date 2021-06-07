#pragma once
#include "WebSocketCommon.hpp"
/*
Ԫ����ģ�飬����һ�����ӣ���������˵����������е����ݣ�ÿһ������Ӿ��Ὺ��һ���߳̽������ݴ�����ͨѶ
Ŀǰ�����þ��ǣ�һ�����ú���method��ô�ö�Ӧ���ӵ�����ͨѶ���������úõĲ��Խ���
*/

class metaConnection {
public:
	enum class connectionStatus {
		OPEN,
		CLOSE,
	};
	enum class Method {
		UNDEFINED,
		IMG_RETRANSMISSION,//Ŀǰ��֧�ִ˷�������
	};
	metaConnection() = delete;
	/*
	MSG_MAX: �����������������������������������������ջ���
	*/
	metaConnection(server& ser, websocketpp::connection_hdl hdl, const int& MSG_MAX = 1000, Method a = Method::UNDEFINED) :m_hdl(hdl), m_status(connectionStatus::OPEN), m_method(a), m_endPoint(ser), FRAMEPOOLMAXSIZE(MSG_MAX) {
		m_alias = "undefined";//�ǳƣ�������δ����״̬��
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
				//����ȫ��
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
	std::list<websocketpp::lib::weak_ptr<metaConnection>>m_toMetaCon;//��Ҫ����ͨѶ������
	Method m_method;
	websocketpp::connection_hdl m_hdl;//�������������
private:
	std::thread m_thread;
	bool m_thrEndLabel;
	server& m_endPoint;
	const int FRAMEPOOLMAXSIZE;
	std::list<std::string>m_framePool;
	std::string m_alias;//��ʼΪundefined
};
