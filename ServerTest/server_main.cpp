#include "WebSocketCommon.hpp"
#include "ServerEndPoint.hpp"
#include "metaConnection.hpp"
//Ŀǰ֧�ֵ�����
/*
"ws://localhost:9106/?Alias=XXXX" -- ��ʼ��ʹ�� �ɹ��᷵��: "?SysConnectInit=Got" ��ʧ�ܹر�����ͬʱ���ش����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
��������ʽ:?SysSetMethod=xxx&FromAlias=xxx&ToAlias=xx,xx,xx----SysSetMethod:��������,FromAlias: ������ǳ�, ToAlias :��Ҫ�����������ǳ� , ���FromeAlias��ToAlias�����ڣ�������ǽ��գ�
            ����Ӹý���ת��
             ��������:?SysSetMethodEcho:xx --------SysSetMethodEcho:OK/FAIL      һ������OK���ID��Ӧ�Ĵ���ʽת��Ϊ�趨�ķ�ʽ
  Ŀǰ֧�ֵķ�ʽ:ImgRetransmission(ͼ��ת��)


//�������̾��ǣ�
1.��ʼ��2.���ô���ʽ 3. �����������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
int main() {
    ServerEndPoint s(20);
    s.run(9106);
    return 0;
}