# MMORPG游戏制作笔记

## 一、服务器制作

### 1、IDEFINE.h文件

```c++
#ifndef __IDEFINE_H
#define __IDEFINE_H

#include <vector>

#ifdef ____WIN32_
#include <winsock2.h>
#pragma comment(lib,"ws2_32") 
#endif

#define MAX_USER_SOCKETFD   1000000
#define MAX_EXE_LEN         200      
#define MAX_FILENAME_LEN    250     
#define SIO_KEEPALIVE_VALS  IOC_IN | IOC_VENDOR | 4 
#define MAX_MD5_LEN  35 
#define MAX_IP_LEN   20 
#define MAX_COMMAND_LEN     65535
#define MAX_IP_ONE_COUNT    20

#define LOG_MSG printf 

#define CMD_HEART     60000
#define CMD_RCODE     65001
#define CMD_SECURITY  65002

#ifdef ____WIN32_
#define RELEASE_POINTER(p)	{if(p != NULL) {delete p; p = NULL;}}
#define RELEASE_HANDLE(h)	{if(h != NULL && h != INVALID_HANDLE_VALUE) { CloseHandle(h); h = INVALID_HANDLE_VALUE; }}
#define RELEASE_SOCKET(s)	{if(s != INVALID_SOCKET) { closesocket(s); s = INVALID_SOCKET; }}
#endif

typedef signed char          s8;
typedef signed short         s16;
typedef signed int           s32;
typedef signed long long     s64;
typedef unsigned char        u8;
typedef unsigned short       u16;
typedef unsigned int         u32;
typedef unsigned long long   u64;
typedef float				 f32;
typedef double				 f64;


namespace func
{

	enum SOCKET_CLOSE
	{
		S_CLOSE_FREE = 0,
		S_CLOSE_ACCEPT = 1,   //连接出错关闭
		S_CLOSE_SHUTDOWN = 2, //关闭连接
		S_CLOSE_CLOSE = 3     //正式关闭
	};


	enum SOCKET_CONTEXT_STATE
	{
		SC_WAIT_ACCEPT = 0,
		SC_WAIT_RECV = 1,
		SC_WAIT_SEND = 2,
		SC_WAIT_RESET = 3,
	};


	//服务器
	enum S_SOCKET_STATE
	{
		S_Free = 0,
		S_Connect = 1,
		S_ConnectSecure = 2,
		S_Login = 3,
		S_NeedSave = 4,
		S_Saving = 5
	};


	//客户端SOCKET枚举状态
	enum C_SOCKET_STATE
	{
		C_Free = 0,
		C_ConnectTry = 1,
		C_Connect = 2,
		C_ConnectSecure = 3,
		C_Login = 4
	};


	//0 玩家 1-DB 2-中心服务器 3-游戏服务器 4-网关服务器 5-登录服务器
	enum E_SERVER_TYPE
	{
		S_TYPE_USER = 0x00,
		S_TYPE_DB,
		S_TYPE_CENTER,
		S_TYPE_GAME,
		S_TYPE_GATE,
		S_TYPE_LOGIN
	};


	struct ConfigXML
	{
		s32   ID;     //服务器ID
		u8    Type;   //服务器类型 1-DB 2-中心服务器 3-地图服务器 4-网关服务器 
		u16   Port;   //服务器端口号
		s32   MaxPlayer;//最大玩家数量
		s32   MaxConnect;//最大客户端连接数量
		u8    RCode;
		s32   Version;
		s32   ReceOne;
		s32   ReceMax;
		s32   SendOne;
		s32   SendMax;
		s32   HeartTime;//心跳时间
		s32   AutoTime;//自动重连时间
		s32   MaxAccpet; //最大投递连接数量
		s32   MaxRece;   //最大收到消息数量
		s32   MaxSend;   //最大发送信息数量
		char  SafeCode[20];
		char  Head[3];
		char  IP[MAX_IP_LEN];
	};


	struct ServerListXML
	{
		s32   ID;
		u16   Port;
		char  IP[MAX_IP_LEN];
	};


	extern char FileExePath[MAX_EXE_LEN];
	extern ConfigXML* __ServerInfo;
	extern ConfigXML* __ClientInfo;
	extern std::vector<ServerListXML*> __ServerListInfo;
	extern u8 GetServerType(s32 sid);
	extern void(*MD5str)(char* output, unsigned char* input, int len);
	extern bool InitData();

	extern const char* getShutDownError(int id);
	extern const char* getCloseSocketError(int id);
	extern void setConsoleColor(u16 index);


}


#endif // !__IDEFINE_H
```

### 2、INetbase.h文件

```c++
#ifndef  __INETBASE_H
#define  __INETBASE_H

#include "IDefine.h"

namespace net
{

#pragma pack(push,packing)
#pragma pack(1)


	//客户端连接数据索引
	struct S_CLIENT_BASE_INDEX
	{
		s32 index;
		inline void Reset() { index = -1; }
	};


	//客户端连接数据
	struct S_CLIENT_BASE
	{
		//固定数据
#ifdef ____WIN32_
		SOCKET       socketfd;
#else
		int          socketfd;
#endif 

		//动态变化 
		s8              state;//用户状态 0空闲 1连接 2登录
		s8              closeState;//关闭状态 必须完成端口队列 没有投递才安全关闭
		char            ip[MAX_IP_LEN];
		u16			    port;
		s32				ID; //对应索引号,由int 改为s32
		u8              rCode;

		//游戏记录数据
		s32             clientID;   //各个功能服务器ID 
		u8              clientType; //0 1-DB 2-Center 3-Game 4-Gate 5-Login 

		//接收数据
		//生产者接收数据
		//消费者解析数据
		bool			is_RecvCompleted;
		char*			recvBuf;
		//char*			recvBuf_Temp;
		s32				recv_Head;//头 消费者使用
		s32				recv_Tail;//尾 生产者使用
		s32				recv_TempHead;//临时头
		s32				recv_TempTail;//临时尾

		//发送数据
		//生产者封包
		//消费者发送数据
		bool			is_Sending;
		bool            is_SendCompleted;
		char*			sendBuf;
		s32				send_Head;
		s32				send_Tail;
		s32				send_TempTail;

		//时间类
		s32             time_Connet;
		s32             time_Heart;
		s32             time_Close;
		u8              threadID;
		s32             shutdown_kind;
		char            md5[MAX_MD5_LEN];
		void Init();
		void Reset();


#ifdef ____WIN32_
		inline bool isT(SOCKET client)
		{
			if (socketfd == client) return true;
			return false;
		}
#else
		inline bool isT(int client)
		{
			if (socketfd == client) return true;
			return false;
		}
#endif
	};

	//************************************************************
	//客户端-结构体
	struct S_SERVER_BASE
	{
#ifdef ____WIN32_
		SOCKET       socketfd;
#else
		s32          socketfd;
#endif 
		s32				ID;   //客户端ID
		char			ip[16];
		u16		        port;
		s32             serverID;//0 1000-DB 2000-Center 3000-Game 4000-Gate 5000-Login 
		u8              serverType;//0 1-DB 2-Center 3-Game 4-Gate 5-Login
									//动态变化字段
		u8              state;//用户状态 0空闲 1尝试连接 2安全连接  3连接成功
		u8              rCode;

		//客户端接收数据
		char*			recvBuf_Temp;
		char*			recvBuf;
		s32				recv_Head;//头 消费者使用
		s32				recv_Tail;//尾 生产者使用
		s32				recv_TempHead;//临时头
		s32				recv_TempTail;//临时尾
		bool			is_Recved;
		

		//客户端发送数据
		char*			sendBuf;
		s32				send_Head;
		s32				send_Tail;
		s32				send_TempTail;
		bool			is_Sending;
		bool            is_SendCompleted;

		//时间类
		s32             time_Heart;
		s32             time_AutoConnect;
		char            md5[MAX_MD5_LEN];

		void Init(int sid);
		void reset();
	};

#pragma pack(pop, packing)


	class ITcpServer;
	class ITcpClient;
	typedef void(*TCPSERVERNOTIFY_EVENT) (ITcpServer* tcp, S_CLIENT_BASE* c, const s32 code);
	typedef void(*TCPSERVERNOTIFYERRO_EVENT) (ITcpServer* tcp, S_CLIENT_BASE* c, const s32 code, const char* err);
	typedef void(*TCPCLIENTNOTIFY_EVENT)(ITcpClient* tcp, const s32 code);


	//定义我们的服务器接口 纯虚函数 
	class ITcpServer
	{
	public:

		virtual ~ITcpServer() {}
		virtual void  runServer(s32 num) = 0;
		virtual void  stopServer() = 0;

#ifdef ____WIN32_

		virtual S_CLIENT_BASE* client(SOCKET socketfd, bool isseriuty) = 0;
#else

		virtual S_CLIENT_BASE* client(int socketfd, bool isseriuty) = 0;
#endif

		virtual S_CLIENT_BASE* client(int index) = 0;

		//解包
		virtual bool  isID_T(const s32 id) = 0;
		virtual bool  isSecure_T(const s32 id, s32 secure) = 0;
		virtual bool  isSecure_F_Close(const s32 id, s32 secure) = 0;

		virtual void  parseCommand() = 0;
		virtual void  getSecurityCount(int& connum, int& securtiynum) = 0;

		//封装发送数据包
		virtual void  begin(const int id, const u16 cmd) = 0;
		virtual void  end(const int id) = 0;
		virtual void  sss(const int id, const s8 v) = 0;
		virtual void  sss(const int id, const u8 v) = 0;
		virtual void  sss(const int id, const s16 v) = 0;
		virtual void  sss(const int id, const u16 v) = 0;
		virtual void  sss(const int id, const s32 v) = 0;
		virtual void  sss(const int id, const u32 v) = 0;
		virtual void  sss(const int id, const s64 v) = 0;
		virtual void  sss(const int id, const u64 v) = 0;
		virtual void  sss(const int id, const bool v) = 0;
		virtual void  sss(const int id, const f32 v) = 0;
		virtual void  sss(const int id, const f64 v) = 0;
		virtual void  sss(const int id, void* v, const u32 len) = 0;

		//解析接收数据包
		virtual void  read(const int id, s8& v) = 0;
		virtual void  read(const int id, u8& v) = 0;
		virtual void  read(const int id, s16& v) = 0;
		virtual void  read(const int id, u16& v) = 0;
		virtual void  read(const int id, s32& v) = 0;
		virtual void  read(const int id, u32& v) = 0;
		virtual void  read(const int id, s64& v) = 0;
		virtual void  read(const int id, u64& v) = 0;
		virtual void  read(const int id, bool& v) = 0;
		virtual void  read(const int id, f32& v) = 0;
		virtual void  read(const int id, f64& v) = 0;
		virtual void  read(const int id, void* v, const u32 len) = 0;

		virtual void  setOnClientAccept(TCPSERVERNOTIFY_EVENT event) = 0;
		virtual void  setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event) = 0;
		virtual void  setOnClientDisconnect(TCPSERVERNOTIFY_EVENT event) = 0;
		virtual void  setOnClientTimeout(TCPSERVERNOTIFY_EVENT event) = 0;
		virtual void  setOnClientExcept(TCPSERVERNOTIFY_EVENT event) = 0;
		virtual void  registerCommand(int cmd, void* container) = 0;

	};


	//客户端纯虚函数
	class ITcpClient
	{
	public:
		virtual ~ITcpClient() {}

		virtual S_SERVER_BASE* getData() = 0;
#ifdef ____WIN32_
		virtual SOCKET getSocket() = 0;
#else
		virtual int getSocket() = 0;
#endif 
		virtual void runClient(u32 sid, char* ip, int por) = 0;
		virtual bool connectServer() = 0;
		virtual void disconnectServer(const s32 errcode, const char* err) = 0;

		virtual void  begin(const u16 cmd) = 0;
		virtual void  end() = 0;
		virtual void  sss(const s8 v) = 0;
		virtual void  sss(const u8 v) = 0;
		virtual void  sss(const s16 v) = 0;
		virtual void  sss(const u16 v) = 0;
		virtual void  sss(const s32 v) = 0;
		virtual void  sss(const u32 v) = 0;
		virtual void  sss(const s64 v) = 0;
		virtual void  sss(const u64 v) = 0;
		virtual void  sss(const bool v) = 0;
		virtual void  sss(const f32 v) = 0;
		virtual void  sss(const f64 v) = 0;
		virtual void  sss(void* v, const u32 len) = 0;

		virtual void  read(s8& v) = 0;
		virtual void  read(u8& v) = 0;
		virtual void  read(s16& v) = 0;
		virtual void  read(u16& v) = 0;
		virtual void  read(s32& v) = 0;
		virtual void  read(u32& v) = 0;
		virtual void  read(s64& v) = 0;
		virtual void  read(u64& v) = 0;
		virtual void  read(bool& v) = 0;
		virtual void  read(f32& v) = 0;
		virtual void  read(f64& v) = 0;
		virtual void  read(void* v, const u32 len) = 0;

		virtual void  parseCommand() = 0;
		virtual void  registerCommand(int cmd, void* container) = 0;
		virtual void  setOnConnect(TCPCLIENTNOTIFY_EVENT event) = 0;
		virtual void  setOnConnectSecure(TCPCLIENTNOTIFY_EVENT event) = 0;
		virtual void  setOnDisconnect(TCPCLIENTNOTIFY_EVENT event) = 0;
		virtual void  setOnExcept(TCPCLIENTNOTIFY_EVENT event) = 0;

	};

	extern ITcpServer* NewTcpServer();
	extern ITcpClient* NewTcpClient();
}



#endif // !____INETBASE_H

```

