#include "WebSocketCommon.hpp"
#include "ServerEndPoint.hpp"
#include "metaConnection.hpp"
//目前支持的命令
/*
"ws://localhost:9106/?Alias=XXXX" -- 初始化使用 成功会返回: "?SysConnectInit=Got" ，失败关闭连接同时返回错误号
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
调整处理方式:?SysSetMethod=xxx&FromAlias=xxx&ToAlias=xx,xx,xx----SysSetMethod:方法名称,FromAlias: 自身的昵称, ToAlias :需要传输给对象的昵称 , 如果FromeAlias和ToAlias不存在，则仅仅是接收，
            不会从该进行转发
             返回类型:?SysSetMethodEcho:xx --------SysSetMethodEcho:OK/FAIL      一旦返回OK则该ID对应的处理方式转变为设定的方式
  目前支持的方式:ImgRetransmission(图像转发)


//正常流程就是：
1.初始化2.设置处理方式 3. 发送相关数据
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
int main() {
    ServerEndPoint s(20);
    s.run(9106);
    return 0;
}