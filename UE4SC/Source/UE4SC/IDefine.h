#ifndef __IDEFINE_H
#define __IDEFINE_H

#include "CoreMinimal.h"
#include "Containers/UnrealString.h"

#define MAX_MD5_LEN   35 
#define CMD_HEART     60000
#define CMD_RCODE     65001
#define CMD_SECURITY  65002


namespace func
{
	//客户端SOCKET枚举状态
	enum C_SOCKET_STATE
	{
		C_Free = 0,
        C_ConnectTry = 1,
        C_Connect = 2,
        C_ConnectSecure = 3,
        C_Login = 4
    };

	struct ConfigXML
	{
		int32   ID;     //服务器ID
		uint8   Type;   //服务器类型 1-DB 2-中心服务器 3-地图服务器 4-网关服务器 
		uint8   RCode;
		int32   Version;
		int32   ReceOne;
		int32   ReceMax;
		int32   SendOne;
		int32   SendMax;
		int32   HeartTime;//心跳时间
		int32   AutoTime;//自动重连时间
		FString SafeCode;
		uint8   Head[2];

	};

	extern ConfigXML* __ClientInfo;

}


namespace net
{

	struct S_SERVER_BASE
	{

		int32				ID;   //客户端ID
		FString				ip;
		uint16		        port;
		int32				serverID;//0 1000-DB 2000-Center 3000-Game 4000-Gate 5000-Login 
		uint8				serverType;//0 1-DB 2-Center 3-Game 4-Gate 5-Login
		//动态变化字段
		uint8				state;//用户状态 0空闲 1尝试连接 2安全连接  3连接成功
		uint8				rCode;

		//客户端接收数据		生产者接收数据
		uint8* recvBuf_Temp;
		uint8* recvBuf;
		int32				recv_Head;//头 消费者使用
		int32				recv_Tail;//尾 生产者使用
		int32				recv_TempHead;//临时头
		int32				recv_TempTail;//临时尾
		bool				is_Recved;


		//客户端发送数据		消费者发送数据
		uint8* sendBuf;
		int32				send_Head;
		int32				send_Tail;
		int32				send_TempTail;
		bool				is_Sending;
		bool				is_SendCompleted;

		//时间类
		int32				time_Heart;
		int32				time_HeartTime;
		int32				time_AutoConnect;
		FString				md5;

		void init(int32 sid);
		void reset();
	};

}

#endif // !__IDEFINE_H