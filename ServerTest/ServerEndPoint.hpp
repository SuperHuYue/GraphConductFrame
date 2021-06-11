#pragma once
#include "WebSocketCommon.hpp"
#include "metaConnection.hpp"
#include "utility/utility.hpp"
#include <unordered_map>

const int CLIENT_FULL_ERR = -1;
const int ALIAS_DUPLICATE = -2;

struct ControlModule {
    ControlModule(server& ser):m_endpoint(ser),m_thrEndLabel(false)
    {
        m_thrConMaintain = std::thread([&]() {
            while (true)
            {
                {
					std::lock_guard lk(m_lock);
					if (m_thrEndLabel) {
						break;
					}
					
					for (auto i : m_hdl2con) {
						auto con = i.second;
						//ά����������״̬������
						//�ⲿ��������̬����Żᱻά��
						if (con->getStatus() == metaConnection::connectionStatus::OPEN) {
							auto innerMetaCon = con->getMetaCon();
							for (auto j = innerMetaCon.begin(); j != innerMetaCon.end(); ++j) {
								//��������̬���Ƿ���close״̬���� 
								if ((*j)->getStatus() == metaConnection::connectionStatus::CLOSE) {
									std::string name;
									if ((*j)->getName(name)) {
										//���Ҹ����ֵ��ⲿ�����Ƿ�Ϊopen����Ϊopen��������¶�����룬��Ҫ����
										auto out = queryConUsingAlisa(name);
										if (out.first) {
											if (out.second->getStatus() == metaConnection::connectionStatus::OPEN) {
												// �滻
												con->replaceMetaCon(name, out.second);
											}
										}
										else {
											continue;
										}
									}
									else {
										continue;//�������ֳ���
									}
								}
							}
						}
						//ά����Ҫ�������ӵĲ��֣�����Ŀ���������ʱ����������ӵĶ�����δ���������
						auto tmp = con->getNeedConnectAlias();
						for (auto itr_name = tmp.begin(); itr_name != tmp.end(); ++itr_name) {
                            auto out = queryConUsingAlisa(*itr_name);
                            if (out.first) {
                                // ������Ȼ����,����
                                i.second->addMetaCon(out.second);
                                // �����������needConnectAlias���Ƴ�
                                con->rmNeedConnectAlias(*itr_name);
                            }
						}
					}

				}

                std::this_thread::yield();
            }
		});
    };
    ~ControlModule() {
        stopThr();
    };
    void stopThr(){
        m_thrEndLabel = true;
        if (m_thrConMaintain.joinable()) {
            m_thrConMaintain.join();
        }
    }

    //����ID�Ų��Ҵ洢����,ÿһ�������µ����Ӷ�����ִ�connection���м��, ���������������Ƿ�close���Ϊclose״̬����ջ���
    inline std::pair<bool, int> AddConnection(std::string& alias, websocketpp::connection_hdl hdl)
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        clearCon(alias);
        std::lock_guard< std::recursive_mutex> lk(m_lock);
        if (m_alias2con.find(alias) == m_alias2con.end())
        {
            m_alias2con[alias] = websocketpp::lib::shared_ptr<metaConnection>(new metaConnection(m_endpoint, hdl));
            m_hdl2con[hdl] = m_alias2con[alias];
            m_hdl2con[hdl]->setName(alias);
        }
        else
        {
            return std::make_pair(false, ALIAS_DUPLICATE);
        }
        return std::make_pair(true, 0);
    }
    inline std::pair<bool, websocketpp::lib::shared_ptr<metaConnection>> queryConUsingAlisa(const std::string& alisa)
    {
        std::lock_guard< std::recursive_mutex> lk(m_lock);
        if (m_alias2con.find(alisa) != m_alias2con.end())
        {
            return std::make_pair(true, m_alias2con[alisa]);
        }
        return std::make_pair(false, nullptr);
    }
    //���趨Ϊ������У����������ն�Ӧalias������
    inline void clearCon(const std::string alias = "undefined")
    {
        std::lock_guard< std::recursive_mutex> lk(m_lock);
        if (!alias.compare("undefined"))
        {
            //�������close������
            for (auto i : m_alias2con)
            {
                auto con = i.second;
                if (m_hdl2con.find(con->getHdl()) != m_hdl2con.end())
                {
                    if (con->getStatus() == metaConnection::connectionStatus::CLOSE)
                    {
                        m_hdl2con.clear();
                        m_alias2con.clear();
                    }
                }
            }
        }
        else
        {
            if (m_alias2con.find(alias) != m_alias2con.end())
            {
                auto sin_con = m_alias2con[alias];
                if (m_hdl2con.find(sin_con->getHdl()) != m_hdl2con.end())
                {
                    if (sin_con->getStatus()== metaConnection::connectionStatus::CLOSE)
                    {
                        m_hdl2con.erase(sin_con->getHdl());
                        m_alias2con.erase(alias);
                    }
                }
                else
                {
                    throw std::runtime_error("Err no clearCon don't have equal...");
                }
            }
        }
    }

    inline std::pair<bool, websocketpp::lib::shared_ptr<metaConnection>>
        queryConUsingHdl(websocketpp::connection_hdl hdl)
    {
        std::lock_guard< std::recursive_mutex> lk(m_lock);
        if (m_hdl2con.find(hdl) != m_hdl2con.end())
        {
            return std::make_pair(true, m_hdl2con[hdl]);
        }
        return std::make_pair(false, nullptr);
    }
    //ά�������̣߳�����ά�����������״̬�Լ��¼���Ķ���Ĵ���,Ŀǰ������ѯ�ķ�ʽ���У�����Ч������
    std::thread m_thrConMaintain;
    bool m_thrEndLabel;
    //
    server& m_endpoint;
     std::recursive_mutex m_lock;                                                                         //����ά��m_alias2con��m_hdl2con
    std::unordered_map<std::string, websocketpp::lib::shared_ptr<metaConnection>> m_alias2con; //������con
    std::map<websocketpp::connection_hdl, websocketpp::lib::shared_ptr<metaConnection>, std::owner_less<websocketpp::connection_hdl>> m_hdl2con;
};




class ServerEndPoint
{
public:
    ServerEndPoint(const int &MAX_CONNECT = 200):m_control(m_endpoint)
    {
        //regist handler
        //websocketpp::lib::shared_ptr<ServerGate> m_serverGate(new ServerGate(m_endpoint, MAX_CONNECT,  m_memCon));
        m_endpoint.set_open_handler(websocketpp::lib::bind(&ServerEndPoint::on_open, this, websocketpp::lib::placeholders::_1));
        m_endpoint.set_validate_handler(websocketpp::lib::bind(&ServerEndPoint::on_validate, this, websocketpp::lib::placeholders::_1));
        m_endpoint.set_message_handler(websocketpp::lib::bind(&ServerEndPoint::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        m_endpoint.set_close_handler(websocketpp::lib::bind(&ServerEndPoint::on_close, this, websocketpp::lib::placeholders::_1));

        // Set logging settings
        //m_endpoint.set_error_channels(websocketpp::log::elevel::all);
        //m_endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);//control console message
        m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);

        // Initialize Asio
        m_endpoint.init_asio();
    }
    ~ServerEndPoint()
    {
        m_endpoint.stop_perpetual();
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void on_open(websocketpp::connection_hdl hdl)
    {
        //���ӽ����ɹ�
        auto con = m_endpoint.get_con_from_hdl(hdl);
        auto request = con->get_request();
        auto url = request.get_uri();
        auto keyValue = getKeyValuePair(url);
        std::string alias;
        if (keyValue.find("Alias") != keyValue.end())
        {
            alias = keyValue["Alias"].front();
        }
        auto out = m_control.AddConnection(alias, hdl);
        if (!out.first)
        {
            std::ostringstream str;
            str << "OnOpen Err: " << out.second << std::endl;
            con->close(websocketpp::close::status::going_away, str.str());
            return;
        }
        websocketpp::lib::error_code ec;
        m_endpoint.send(hdl, "?SysConnectInit=Got", websocketpp::frame::opcode::text, ec);
        if (ec)
        {
            std::cout << "> Error send " << out.second << ": "
                      << ec.message() << std::endl;
        }
    }

    bool on_validate(websocketpp::connection_hdl hdl)
    {
        return true;
    }

    //on_message not block
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg)
    {
        if (sysMessageconduct(hdl, msg))
            return;
        auto con = m_control.queryConUsingHdl(hdl);
        if (con.first)
        {
            auto meta = con.second;
            meta->feedFrame(msg->get_raw_payload());
        }
        else
        {
            auto con = m_endpoint.get_con_from_hdl(hdl);
            con->close(websocketpp::close::status::abnormal_close, "err:can't find queryConUsingHdl...Check program...");
        }
    }

    void on_close(websocketpp::connection_hdl hdl)
    {
        auto out = m_control.queryConUsingHdl(hdl);
        if (out.first) {
            out.second->setStatus(metaConnection::connectionStatus::CLOSE);
        }
        return;
	}

    /////////////////////////////////////////////////////////////////////////////////////////
    //

    /////////////////////////////////////////////////////////////////////////////////////////
    void run(const int &port)
    {
        // Listen on port
        m_endpoint.listen(port);
        // Queues a connection accept operation
        m_endpoint.start_accept();
        // Start the Asio io_service run loop
        m_endpoint.run();
    }
    server m_endpoint;
private:
    bool sysMessageconduct(websocketpp::connection_hdl hdl, server::message_ptr msg)
    {
        if (msg->get_opcode() == websocketpp::frame::opcode::TEXT)
        {
            auto key_value_pair = getKeyValuePair(msg->get_payload());
            //SysCommand check just check the first one
            if (key_value_pair.size() == 0)
            {
                return false;
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////
            {
                // /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                const std::string command = "SysSetMethod";
                if (key_value_pair.find(command) != key_value_pair.end())

                {
                    /*  key_value_pair.find("FromAlisa") != key_value_pair.end() &&
                      key_value_pair.find("ToAlisa") != key_value_pair.end()*/
                    auto method = key_value_pair[command].front();
                    //auto fromCon = conList.front();
                    auto fromCon = m_control.queryConUsingHdl(hdl);
                    std::ostringstream str;
                    if (fromCon.first)
                    {
                        fromCon.second->SetMethod(method);
                        if (key_value_pair.find("FromAlisa") != key_value_pair.end() &&
                            key_value_pair.find("ToAlisa") != key_value_pair.end())
                        {
                            auto toAlisa = key_value_pair["ToAlisa"];
                            for (auto i : toAlisa)
                            {
                                auto sinToCon = m_control.queryConUsingAlisa(i);
                                if (sinToCon.first)
                                {
                                    fromCon.second->addMetaCon(sinToCon.second);
                                }
                                else {
                                    //�趨֮ʱ��Ŀ����δ��������,����needģ�� 
                                    fromCon.second->addNeedConnectAlias(i);
                                }
                            }
                        }
                        str << "?SysSetMethod:OK";
                        //SendTo(str.str(), *fromCon.second);
                        fromCon.second->SendTo(str.str());
                    }
                    else
                    {
                        //  ��һ��Ӧ���޷�����
                        websocketpp::lib::error_code ec;
                        str << "SysSetMethodEcho:FAIL";
                        //SendTo(str.str(), *fromCon.second);
                        fromCon.second->SendTo(str.str());
                    }
                    return true;
                }
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////
        }
        return false;
    }
    ControlModule m_control;
};
