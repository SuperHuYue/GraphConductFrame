#pragma once
#include "WebSocketCommon.hpp"
/*
Ԫ����ģ�飬����һ�����ӣ���������˵����������е����ݣ�ÿһ������Ӿ��Ὺ��һ���߳̽������ݴ�����ͨѶ
Ŀǰ�����þ��ǣ�һ�����ú���method��ô�ö�Ӧ���ӵ�����ͨѶ���������úõĲ��Խ���
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
		IMG_RETRANSMISSION,//Ŀǰ��֧�ִ˷�������
	};
	metaConnection() = delete;
	/*
	MSG_MAX: �����������������������������������������ջ���
	*/
	metaConnection(server& ser, websocketpp::connection_hdl hdl, const int& MSG_MAX = 1000, Method a = Method::UNDEFINED) :m_hdl(hdl), m_status(connectionStatus::OPEN), m_method(a), m_endpoint(ser), FRAMEPOOLMAXSIZE(MSG_MAX) {
		m_alias = "undefined";//�ǳƣ�������δ����״̬��
		m_thrEndLabel = false;
		start();
	};
	~metaConnection() {
		//�߳�join֮�����ڴ�join�˴�����Ϊ�����ط����ܵ���stop
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
		std::lock_guard lk(m_lock);
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
		std::lock_guard lk(m_lock);
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
		std::lock_guard lk(m_lock);
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
	inline void setStatus(connectionStatus status) {
		std::lock_guard lk(m_lock);
		m_status = status;
	}
	inline connectionStatus getStatus() {
		std::lock_guard lk(m_lock);
		return m_status;
	}
	inline websocketpp::connection_hdl getHdl() {
		std::lock_guard lk(m_lock);
		return m_hdl;
	}

	inline void addMetaCon(websocketpp::lib::shared_ptr<metaConnection> a) {
		std::lock_guard lk(m_lock);
		m_toMetaCon.push_back(a);
	}
	inline std::list<websocketpp::lib::shared_ptr<metaConnection>> getMetaCon() {
		std::lock_guard lk(m_lock);
		return m_toMetaCon;
	}
	inline void replaceMetaCon(const std::string& alias, websocketpp::lib::shared_ptr<metaConnection>ptr) {
		std::lock_guard lk(m_lock);
		for (auto i = m_toMetaCon.begin(); i != m_toMetaCon.end();++i) {
			std::string name;
			if ((*i)->getName(name)) {
				if (!name.compare(alias)) {
					//���ҵ����� ,�滻
					*i = ptr;
					break;
				}
			}
		}
	}

	/*
	 ���Ҹ��ǳ��Ƿ������needConnectAlias��
	 return value: 1:���� 0û�г���
	*/
	inline bool aliasInNeedConnectAlias(const std::string& alias) {
		std::lock_guard lk(m_lock);
		for (const auto& i : m_needConnectAlias) {
			if (!i.compare(alias)) {
				return true;
			}
		}
		return false;
	}

	inline std::vector<std::string> getNeedConnectAlias() {
		std::lock_guard lk(m_lock);
		return m_needConnectAlias;
	}


	inline void addNeedConnectAlias(const std::string& alias){
		std::lock_guard lk(m_lock);
		//check if exist
		for (const auto& i : m_needConnectAlias) {
			if (!i.compare(alias)) {
				// ��������Ȼ����
				return;
			}
		}
		m_needConnectAlias.push_back(alias);
	}

	/*
	���ǳƴ�needConnectAlias���Ƴ�
	*/
	inline void rmNeedConnectAlias(const std::string& alias) {
		std::lock_guard lk(m_lock);
		for (auto itr_begin = m_needConnectAlias.begin(); itr_begin != m_needConnectAlias.end(); itr_begin++) {
			if (!alias.compare(*itr_begin)) {
				m_needConnectAlias.erase(itr_begin);
				break;
			}
		}
		return;
	}
private:
	server& m_endpoint;
	const int FRAMEPOOLMAXSIZE;
	std::mutex m_lock;
	connectionStatus m_status;
	std::thread m_thread;
	bool m_thrEndLabel;
	std::list<std::string>m_framePool;
	std::string m_alias;//��ʼΪundefined,�����Լ�������
	std::list<websocketpp::lib::shared_ptr<metaConnection>>m_toMetaCon;//��Ҫ����ͨѶ������
	std::vector<std::string> m_needConnectAlias;// ���趨������ʱ��,���ǳƵĶ�����δ�������磬����Controlģ��Ժ����������ά��
	Method m_method;
	websocketpp::connection_hdl m_hdl;//�������������
};
