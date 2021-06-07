#pragma once
#include "WebSocketCommon.hpp"
#include "metaConnection.hpp"
#include "utility/utility.hpp"
#include <unordered_map>

const int CLIENT_FULL_ERR = -1;
const int ALIAS_DUPLICATE = -2;

class ServerEndPoint
{
public:
    ServerEndPoint(const int &MAX_CONNECT = 200)
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
        //连接建立成功
        auto con = m_endpoint.get_con_from_hdl(hdl);
        auto request = con->get_request();
        auto url = request.get_uri();
        auto keyValue = getKeyValuePair(url);
        std::string alias;
        if (keyValue.find("Alias") != keyValue.end())
        {
            alias = keyValue["Alias"].front();
        }
        auto out = AddConnection(alias, hdl);
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
        auto con = queryConUsingHdl(hdl);
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
        std::lock_guard<std::mutex> lk(m_lock);
        //check which one close
        if (m_hdl2con.find(hdl) != m_hdl2con.end())
        {
            m_hdl2con[hdl]->m_status = metaConnection::connectionStatus::CLOSE; //仅仅做关闭flag
        }
        return;
        //定期维护连接状态
        //while (true) {
        //    bool collected = false;
        //    for (auto& i : m_id2Obj) {
        //        auto& con_obj = i.second;
        //        auto sin_con = m_s.get_con_from_hdl(con_obj->get_hdl());
        //        if (websocketpp::session::state::value::closed == sin_con->get_state()) {
        //            m_id2Obj[i.first]->m_status = connection_metadata::status::CLOSE;
        //            std::string tmp = i.second->GetName();
        //            addRetrieveIdPool(i.first, tmp);
        //            collected = true;
        //            break;
        //        }
        //    }
        //    if (!collected)break;
        //}
        //end
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
                    auto fromCon = queryConUsingHdl(hdl);
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
                                auto sinToCon = queryConUsingAlisa(i);
                                if (sinToCon.first)
                                {
                                    fromCon.second->m_toMetaCon.push_back(sinToCon.second);
                                }
                            }
                        }
                        str << "?SysSetMethod:OK";
                        //SendTo(str.str(), *fromCon.second);
                        fromCon.second->SendTo(str.str());
                    }
                    else
                    {
                        //  这一段应该无法进入
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

    //分配ID号并且存储内容,每一次增加新的连接都会对现存connection进行检测, 并分析重名内容是否close如果为close状态则清空缓存
    inline std::pair<bool, int> AddConnection(std::string &alias, websocketpp::connection_hdl hdl)
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        clearCon(alias);
        std::lock_guard<std::mutex> lk(m_lock);
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
    inline std::pair<bool, websocketpp::lib::shared_ptr<metaConnection>> queryConUsingAlisa(const std::string &alisa)
    {
        std::lock_guard<std::mutex> lk(m_lock);
        if (m_alias2con.find(alisa) != m_alias2con.end())
        {
            return std::make_pair(true, m_alias2con[alisa]);
        }
        return std::make_pair(false, nullptr);
    }
    //不设定为清空所有，否则仅仅清空对应alias的内容
    inline void clearCon(const std::string alias = "undefined")
    {
        std::lock_guard<std::mutex> lk(m_lock);
        if (!alias.compare("undefined"))
        {
            //清空所有close的连接
            for (auto i : m_alias2con)
            {
                auto con = i.second;
                if (m_hdl2con.find(con->m_hdl) != m_hdl2con.end())
                {
                    if (con->m_status == metaConnection::connectionStatus::CLOSE)
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
                if (m_hdl2con.find(sin_con->m_hdl) != m_hdl2con.end())
                {
                    if (sin_con->m_status == metaConnection::connectionStatus::CLOSE)
                    {
                        m_hdl2con.erase(sin_con->m_hdl);
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
        std::lock_guard<std::mutex> lk(m_lock);
        if (m_hdl2con.find(hdl) != m_hdl2con.end())
        {
            return std::make_pair(true, m_hdl2con[hdl]);
        }
        return std::make_pair(false, nullptr);
    }

    std::mutex m_lock;                                                                         //用于维护m_alias2con和m_hdl2con
    std::unordered_map<std::string, websocketpp::lib::shared_ptr<metaConnection>> m_alias2con; //别名到con
    std::map<websocketpp::connection_hdl, websocketpp::lib::shared_ptr<metaConnection>, std::owner_less<websocketpp::connection_hdl>> m_hdl2con;
    server m_endpoint;
};