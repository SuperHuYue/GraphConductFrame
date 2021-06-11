#include <iostream>
#include "HCNetSDK.h"
#include "plaympeg4.h"
#include "Windows.h" 
//websocket part 
#include <opencv2\opencv.hpp>  
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
typedef websocketpp::client<websocketpp::config::asio_client> client;
client::connection_ptr con = nullptr;
client c;
void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
	if (msg->get_opcode() == websocketpp::frame::opcode::TEXT) {
		std::string data = msg->get_payload();
		if (data.find("Got") != std::string::npos) {
			std::string para_set = "?SysSetMethod=ImgRetransmission&FromAlisa=hk&ToAlisa=william";
			con->send(para_set, websocketpp::frame::opcode::TEXT);
		}
	}
}

//using namespace cv;
using namespace std;
int iPicNum = 0;//Set channel NO.  
LONG nPort = -1;
HWND hWnd = NULL;
int countTest = 0;
std::thread thr;
//CustomServer server(9036);
//解码回调 视频为YUV数据(YV12)，音频为PCM数据  
std::mutex lk;
//std::list<std::vector<uint8_t>> buf;
std::list<cv::Mat>buf;
void CALLBACK DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	long lFrameType = pFrameInfo->nType;

	if (lFrameType == T_YV12)
	{
		std::cout << "Now counts: " << countTest++ << "\n";
		if (con != nullptr) {
			UINT32 col= pFrameInfo->nWidth;
			UINT32 row= pFrameInfo->nHeight;
			UINT32 deepth = 1; //YV12为单通道
			UINT32 pic_length = nSize;
			UINT32 head[9] = { 0 };
			head[0] = 36;
			head[1] = 1;
			head[2] = 16 + nSize;//bodysize= row+col+deep+piclen + img_data
			char* finalData =new char[nSize + head[0] + 12];
			int offset = 0;
			for (int i = 0; i < 9; ++i) {
				offset = i * 4;
				memcpy(finalData + offset, &head[i], sizeof(UINT32));
			}
			memcpy(finalData + offset, &row, sizeof(UINT32));
			offset += 4;
			memcpy(finalData + offset, &col, sizeof(UINT32));
			offset += 4;
			memcpy(finalData + offset, &deepth, sizeof(UINT32));
			offset += 4;
			memcpy(finalData + offset, &pic_length, sizeof(UINT32));
			offset += 4;
			memcpy(finalData + offset, pBuf, nSize);
			offset += nSize;
			try {
			std::string data  = std::string(finalData,offset);
			cv::Mat dst(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);//这里nHeight为720,nWidth为1280,8UC3表示8bit uchar 无符号类型,3通道值
			cv::Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, (uchar*)pBuf);
			cv::cvtColor(src, dst, CV_YUV2BGR_YV12);
			delete[] finalData;
			finalData = nullptr;
			{
				std::lock_guard inlk(lk);
				buf.push_back(dst);
			}
			Sleep(0.1);
			//std::error_code ec=  con->send(finalData, offset,websocketpp::frame::opcode::BINARY);//此方法不仅缓存会很慢
			//con->send(data, websocketpp::frame::opcode::BINARY);
			}
			catch (...) {

			}
		}
	}
}

///实时流回调  
void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
	DWORD dRet = 0;
	BOOL inData = FALSE;
	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD:    //系统头  
		if (nPort >= 0)
		{
			break; //同一路码流不需要多次调用开流接口
		}
		if (!PlayM4_GetPort(&nPort)) //获取播放库未使用的通道号  
		{
			break;
		}
		if (dwBufSize > 0)
		{
			if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME))  //设置实时流播放模式
			{
				cout << "PlayM4_SetStreamOpenMode failed " << endl;
				break;
			}
			if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 1024 * 1024))   //查询
			{
				cout << "PlayM4_OpenStream failed " << endl;
				dRet = PlayM4_GetLastError(nPort);
				break;
			}
			//设置解码回调函数 只解码不显示  
			if (!PlayM4_SetDecCallBack(nPort, DecCBFun))                    //查询
			{
				dRet = PlayM4_GetLastError(nPort);
				break;
			}

			//设置解码回调函数 解码且显示  
			//if (!PlayM4_SetDecCallBackEx(nPort,DecCBFun,NULL,NULL))  
			//{  
			//  dRet=PlayM4_GetLastError(nPort);  
			//  break;  
			//}  

			//打开视频解码  
			if (!PlayM4_Play(nPort, hWnd))
			{
				dRet = PlayM4_GetLastError(nPort);
				break;
			}

			//打开音频解码, 需要码流是复合流  
			/*if (!PlayM4_PlaySound(nPort))
			{
			dRet = PlayM4_GetLastError(nPort);
			break;
			}*/
		}
		break;

	case NET_DVR_STREAMDATA:   //码流数据  
		inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
		while (!inData)
		{
			Sleep(10);
			inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
			cout << "PlayM4_InputData failed 11111" << endl;
			break;
		}
		break;
	default:
		inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
		while (!inData)
		{
			Sleep(10);
			inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
			break;
		}
		break;
	}
}

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	char tempbuf[256] = { 0 };
	switch (dwType)
	{
	case EXCEPTION_RECONNECT:    //预览时重连  
		cout << "----------reconnect--------" << endl;
		break;
	default:
		break;
	}
}
void main()
{
	// 初始化socket
		// 初始化  
	NET_DVR_Init();
	//设置连接时间与重连时间  
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);


	//---------------------------------------  
	// 注册设备  
	LONG lUserID;
	NET_DVR_USER_LOGIN_INFO struLoginInfo = { 0 };
	NET_DVR_DEVICEINFO_V40 struDeviceInfo = { 0 };

	strcpy((char*)struLoginInfo.sDeviceAddress, "192.168.2.156"); //设备 IP 地址
	strcpy((char*)struLoginInfo.sUserName, "admin"); //设备登录用户名
	strcpy((char*)struLoginInfo.sPassword, "cfsoft0000"); //设备登录密码
	struLoginInfo.wPort = 8000;
	struLoginInfo.bUseAsynLogin = 0; //同步登录，登录接口返回成功即登录成功

	lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfo);
	if (lUserID < 0)
	{
		cout << "NET_DVR_Login_V40 failed, error code: " << NET_DVR_GetLastError() << endl;
		NET_DVR_Cleanup();
		return;
	}
	int iRet;
	//获取通道 1 的压缩参数
	DWORD dwReturnLen;
	NET_DVR_COMPRESSIONCFG_V30 struParams = { 0 };
	iRet = NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_COMPRESSCFG_V30, 1, &struParams, \
		sizeof(NET_DVR_COMPRESSIONCFG_V30), &dwReturnLen);
	if (!iRet)
	{
		printf("NET_DVR_GetDVRConfig NET_DVR_GET_COMPRESSCFG_V30 error.\n");
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	//设置通道 1 的压缩参数
	struParams.struNormHighRecordPara.dwVideoBitrate = 0.5;
	iRet = NET_DVR_SetDVRConfig(lUserID, NET_DVR_SET_COMPRESSCFG_V30, 1, \
		& struParams, sizeof(NET_DVR_COMPRESSIONCFG_V30));
	if (!iRet)
	{
		printf("NET_DVR_GetDVRConfig NET_DVR_SET_COMPRESSCFG_V30 error.\n");
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	printf("Video Bitrate is %d\n", struParams.struNormHighRecordPara.dwVideoBitrate);
	//---------------------------------------  
	//设置异常消息回调函数  
	NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);


	NET_DVR_PREVIEWINFO StruPlayInfo = { 0 };
	StruPlayInfo.hPlayWnd = NULL;  //窗口为空，设备SDK不解码只取流  
	StruPlayInfo.lChannel = 1;     //预览通道号
	StruPlayInfo.dwStreamType = 0; //0-主流码，1-子流码，2-流码3，3-流码4，以此类推
	StruPlayInfo.dwLinkMode = 0;   //0-TCP方式，1-UDP方式，2-多播方式，3-RTP方式，4-RTP/RTSP，5-RSTP/HTTP
	StruPlayInfo.bBlocked = 1;     //0-非堵塞取流，1-堵塞取流


	LONG lRealPlayHandle;
	lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &StruPlayInfo, fRealDataCallBack, NULL);


	if (lRealPlayHandle < 0)
	{
		cout << "NET_DVR_RealPlay_V40 failed! Error number:  " << NET_DVR_GetLastError() << endl;
		return;
	}
	//server.start();
	cout << "The program is successful !!" << endl;
	//websocket
	std::string uri = "ws://localhost:9106/?Alias=hk";
	try {
		// Set logging to be pretty verbose (everything except message payloads)
		c.clear_access_channels(websocketpp::log::alevel::all);
		//c.set_access_channels(websocketpp::log::alevel::all);
		//c.clear_access_channels(websocketpp::log::alevel::frame_payload);
		//c.set_error_channels(websocketpp::log::elevel::all);
		// Initialize ASIO
		c.init_asio();

		// Register our message handler
		c.set_message_handler(&on_message);

		websocketpp::lib::error_code ec;
		con = c.get_connection(uri, ec);
		//此处为一写死的情况
	
		if (ec) {
			std::cout << "could not create connection because: " << ec.message() << std::endl;
			return;
		}

		// Note that connect here only requests a connection. No network messages are
		// exchanged until the event loop starts running in the next line.
		c.connect(con);


		thr = std::thread([]() {
			while (true)
			{
				cv::Mat data1;
				{

					std::lock_guard inlk(lk);
					if (!buf.empty()) {
						data1 = buf.back();
						buf.pop_back();
					}
					if (buf.size() > 100) {
						buf.clear();
					}
				}
				if (!data1.empty()) {
					std::vector<unsigned char>buf;
					cv::imencode(".jpeg",data1, buf);
					// con->send(static_cast<void*>(data1.data), data1.size(), websocketpp::frame::opcode::BINARY);
					con->send(buf.data(), buf.size(), websocketpp::frame::opcode::BINARY);
					//con->send((void const* (data1.data)), data1.size(), websocketpp::frame::opcode::BINARY);
					//websocketpp::lib::error_code ec =con->send(vecData.data(),vecData.size(), websocketpp::frame::opcode::BINARY);
				}
				std::this_thread::yield();
			}
		});
		// Start the ASIO io_service run loop
		// this will cause a single connection to be made to the server. c.run()
		// will exit when this connection is closed.
		c.run();



	}
	catch (websocketpp::exception const& e) {
		std::cout << e.what() << std::endl;
	}
	//end

	Sleep(10000);
	//not have a out label yet
//	while (true)
	//{
	//	server.Update(-1, true);
	//}

	//---------------------------------------  
	//关闭预览  
	if (!NET_DVR_StopRealPlay(lRealPlayHandle))
	{
		cout << "NET_DVR_StopRealPlay error! Error number: " << NET_DVR_GetLastError() << endl;
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	//注销用户  
	NET_DVR_Logout(lUserID);
	NET_DVR_Cleanup();

	return;
}




