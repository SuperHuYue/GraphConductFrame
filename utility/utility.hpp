#pragma once
#include <string>
#include <utility>
#include <vector>
#include <list>
#include <map>
#include <set>

/*
 * 函数功能：获得url -key，value对
 * inData:输入数据
 * init_label:开始查找位置, 为空则从头开始
 * divSig：键值对分隔符
 * offserSig：赋值符
 * 返回说明:
 * 包含具体key，value的对象
 * 示例说明：
 *  ”/?Method=william&pass=199067“,所得到的[key,value]分别为["Method","william"],["pass","199067"]
 */
std::map<std::string, std::list<std::string>> getKeyValuePair(std::string inData, 
                                                    char init_label = '?', char divSig = '&',
                                                    char offerSig = '=', bool bInitLabelHave = true, bool inner_div_used = true)
{
    std::map<std::string, std::list<std::string>>out;
    std::vector<char> tmp_cache;
    std::string::size_type n=0;

    auto inner_div = [inner_div_used](std::string in_data, char div = ',')->std::list<std::string>{
        std::list<std::string> ret_val;
        if (!inner_div_used) {
            ret_val.push_back(in_data);
            return std::move(ret_val);
        }

        while (true)
        {
            size_t npos = in_data.find(div);
            std::string tmp_string;
            if(npos == std::string::npos){
                ret_val.push_back(in_data);
                break;
            }
            tmp_string = in_data.substr(0, npos);
            in_data = in_data.substr(npos+1);
            ret_val.push_back(tmp_string);
        }
        return std::move(ret_val);
    };
    if(bInitLabelHave)
        n = inData.find(init_label);
        if(n == std::string::npos)return out;
        inData = inData.substr(n + 1);
    while(true){
        std::string key;
        std::string value;
        n = inData.find(offerSig);
        if(n == std::string::npos)break;
        key = inData.substr(0,n);
        inData = inData.substr(n + 1);
        n = inData.find(divSig);
        if(n == std::string::npos){

            out[key] = inner_div(inData);
            break;
        }
        value = inData.substr(0,n);
        inData = inData.substr(n + 1);
        out[key] = inner_div(value);
    }
    return std::move(out);
}


//thread safe ID generate Machine
//0~MaxNum
class IdNumDeliverMachine{
    public:
    IdNumDeliverMachine(const size_t& MaxNum = 10000):
		m_maxNum(MaxNum)
		, m_nowId(1)
    {

    };
    ~IdNumDeliverMachine(){

    };
    std::pair<bool,size_t> showMeAnID(){
        std::lock_guard<std::mutex> lk(m_IdGenerateMutex);
        size_t id_num = 0;
        if(m_nowId > m_maxNum){
            if(m_garbageCollect.empty())return std::make_pair(false, 0);
            id_num = *m_garbageCollect.begin();
            m_garbageCollect.erase(id_num);
            // m_garbageCollect.pop_front();
            return std::make_pair(true, id_num);
        }
        id_num = m_nowId++;
        return std::make_pair(true, id_num);
    }
    void backAnId(const size_t& id){
        std::lock_guard lk(m_IdGenerateMutex);
        m_garbageCollect.insert(id);
    }
    private:
    std::mutex m_IdGenerateMutex;
    int m_nowId;
    const size_t m_maxNum;
    std::set<size_t>m_garbageCollect;
};
