#include "TcpServer.h"
#ifdef ____WIN32_
#include "IOPool.h"

using namespace func;

namespace net
{
	ITcpServer* net::NewTcpServer()
	{
		return new net::TcpServer();
	}

	TcpServer::TcpServer()
	{
		m_ThreadNum = 0;
		m_AcceptEx = NULL;
		m_GetAcceptEx = NULL;
		listenfd = INVALID_SOCKET;
		m_Completeport = NULL;
		m_ConnectCount = 0;
		m_SecurityCount = 0;
		m_IsRunning = false;

		onAcceptEvent = nullptr;
		onSecureEvent = nullptr;
		onTimeOutEvent = nullptr;
		onDisconnectEvent = nullptr;
		onExceptEvent = nullptr;
	}


	TcpServer::~TcpServer()
	{
	}


	//运行服务器入口
	void TcpServer::runServer(s32 num)
	{
		//1、创建连接用户
		Linkers = new HashArray<S_CLIENT_BASE>(func::__ServerInfo->MaxConnect);
		for (int i = 0; i < Linkers->length; i++)
		{
			S_CLIENT_BASE* client = Linkers->Value(i);
			client->Init();
		}

		//2、连接用户索引
		LinkersIndexs = new HashArray<S_CLIENT_BASE_INDEX>(MAX_USER_SOCKETFD);
		for (int i = 0; i < LinkersIndexs->length; i++)
		{
			S_CLIENT_BASE_INDEX* client = LinkersIndexs->Value(i);
			client->Reset();
		}

		//3、指令集初始化
		initCommands();

		//4、初始化socket
		s32 errcode = initSocket();
		if (errcode < 0)
		{
			LOG_MSG("InitServer err...%d \n", errcode);

			RELEASE_HANDLE(m_Completeport);
			RELEASE_SOCKET(listenfd);
			if (errcode != -2) WSACleanup();
			return;
		}

		//5、初始化投递
		initPost();
		//6、调用工作线程
		this->runThread(num);
	}


	//停止服务器！！！！！！！！！！！！
	void TcpServer::stopServer()
	{
		for (int i = 0; i < m_ThreadNum; i++)
		{
			PostQueuedCompletionStatus(m_Completeport, 0, (DWORD)1, NULL);
		}
		m_IsRunning = false;
		RELEASE_HANDLE(m_Completeport);
		RELEASE_SOCKET(listenfd);
		WSACleanup();
		LOG_MSG("stop server success...\n");
	}


	s32 TcpServer::initSocket()
	{
		//1、创建完成端口
		m_Completeport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (m_Completeport == NULL) return -1;

		//2、初始化Windows Sockets DLL  加载套接字
		WSADATA  wsData;
		int errorcode = WSAStartup(MAKEWORD(2, 2), &wsData);
		if (errorcode != 0) return -2;

		//3、创建套接字 重叠IO WSA_FLAG_OVERLAPPED
		listenfd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (listenfd == INVALID_SOCKET) return -3;

		//4、设置套接字非阻塞模式 控制套接口的模式。
		//允许或禁止套接口s的非阻塞模式
		unsigned long ul = 1;
		errorcode = ioctlsocket(listenfd, FIONBIO, (unsigned long*)&ul);
		if (errorcode == SOCKET_ERROR) return -4;

		//5、关闭监听socket的接收与发送缓冲区
		int size = 0;
		setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(int));
		setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(int));

		//6、把监听socket和完成端口邦定
		HANDLE  handle = CreateIoCompletionPort((HANDLE)listenfd, m_Completeport, (DWORD)func::SC_WAIT_ACCEPT, 0);
		if (handle == nullptr) return -5;

		//7、绑定套接字 
		//htons是将整型变量从主机字节顺序转变成网络字节顺序，
		//就是整数在地址空间存储方式变为高位字节存放在内存的低地址处
		sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(__ServerInfo->Port);
		serAddr.sin_addr.S_un.S_addr = INADDR_ANY;

		errorcode = ::bind(listenfd, (struct sockaddr*)&serAddr, sizeof(serAddr));
		if (errorcode == SOCKET_ERROR) return -6;


		//8、监听套接字 已经完成三次握手的队列数量 定义了系统中每一个端口最大的监听队列的长度,
		errorcode = listen(listenfd, SOMAXCONN);
		if (errorcode == SOCKET_ERROR) return -7;

		//9、 AcceptEx 和 GetAcceptExSockaddrs 的GUID，用于导出函数指针
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		GUID GuidGetAcceptEx = WSAID_GETACCEPTEXSOCKADDRS;
		DWORD dwBytes = 0;

		if (m_AcceptEx == nullptr)
		{
			//WSAIoctl()，控制一个套接口的模式。调用成功后，WSAIoctl()函数返回0。
			//否则的话，将返回SOCKET_ERROR错误，应用程序可通过WSAGetLastError()来获取相应的错误代码。
			errorcode = WSAIoctl(listenfd,			//你需要控制的套接口的句柄。
								SIO_GET_EXTENSION_FUNCTION_POINTER,	//将进行的操作的控制代码，即你所需要操控的类型
								&GuidAcceptEx,						//输入缓冲区的地址（如果你想指定一个函数对套接字进行控制，这里可以是一个guid，指定这个控制函数的guid）。
								sizeof(GuidAcceptEx),				//输入缓冲区的大小（这里为guid的大小，即sizeof(&guid)）。
								&m_AcceptEx,						//输出缓冲区的地址（这里即是函数指针的地址）。
								sizeof(m_AcceptEx),					//输出缓冲区的大小（函数指针的大小）
								&dwBytes,							//输出实际字节数的地址
								NULL,								//WSAOVERLAPPED结构的地址（一般为NULL）。
								NULL);								//一个指向操作结束后调用的例程指针（一般为NULL）。
		}

		if (m_AcceptEx == nullptr || errorcode == SOCKET_ERROR) return -8;

		// 获取GetAcceptExSockAddrs函数指针，也是同理
		if (m_GetAcceptEx == nullptr)
		{
			errorcode = WSAIoctl(listenfd,
				SIO_GET_EXTENSION_FUNCTION_POINTER,
				&GuidGetAcceptEx,
				sizeof(GuidGetAcceptEx),
				&m_GetAcceptEx,
				sizeof(m_GetAcceptEx),
				&dwBytes,
				NULL,
				NULL);
		}
		if (m_GetAcceptEx == nullptr || errorcode == SOCKET_ERROR) return -9;

		return 0;
	}


	//初始化投递
	void TcpServer::initPost()
	{
		//投递连接
		for (int i = 0; i < __ServerInfo->MaxAccpet; i++)
			postAccept();
	}


	s32 TcpServer::postAccept()
	{
		//1、创建一个新的socketfd 
		SOCKET socketfd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (socketfd == INVALID_SOCKET) return -1;

		//2、设置为非阻塞模式
		ULONG ul = 1;
		int errorCode = ioctlsocket(socketfd, FIONBIO, (unsigned long*)&ul);
		if (errorCode == SOCKET_ERROR)
		{
			closeSocket(socketfd, NULL, 1001);			
			return -2;
		}

		//3、获取一个对象回收池里的accept对象
		AcceptContext* context = AcceptContext::pop();
		context->setSocket(listenfd, socketfd);

		//4、投递AcceptEx
		unsigned long dwBytes = 0;

		bool isaccept = m_AcceptEx(context->listenfd,       //1、监听soceket
									context->m_Socket,								//2、受连接socket
									context->m_buf,									//3、接受缓冲区 a、客户端发来第一组数据 b、server地址 c、client地址
									0,												//4、0不会等到数据到来直接返回 非0等待数据
									sizeof(SOCKADDR_IN) + 16,						//5、本地地址大小；长度必须为地址长度 + 16字节
									sizeof(SOCKADDR_IN) + 16,						//6、远端地址大小；长度必须为地址长度 + 16字节
									&dwBytes,										//7、同步方式才有用 我们是异步IO没用，不用管；
									&context->m_OverLapped);						//8、本次重叠I / O所要用到的重叠结构

		if (isaccept == false)
		{
			int error = WSAGetLastError();
			if (ERROR_IO_PENDING != error)
			{
				closeSocket(socketfd, NULL, 1002);
				AcceptContext::push(context);
				return -3;
			}
		}
		return 0;
	}


	s32 TcpServer::onAccept(void* context)
	{

		AcceptContext* acc = (AcceptContext*)context;
		if (acc == nullptr) return -1;

		SOCKADDR_IN* ClientAddr = NULL;
		SOCKADDR_IN* LocalAddr = NULL;
		int remoteLen = sizeof(SOCKADDR_IN);
		int localLen = sizeof(SOCKADDR_IN);
		int errorCode = 0;

		//获取客户端数据地址信息
		m_GetAcceptEx(acc->m_buf,								//1、指向传递给AcceptEx函数接收第一块数据的缓冲区
						0,										//2、缓冲区大小，必须和传递给AccpetEx函数的一致
						sizeof(SOCKADDR_IN) + 16,				//3、本地地址大小，必须和传递给AccpetEx函数一致
						sizeof(SOCKADDR_IN) + 16,				//4、远程地址大小，必须和传递给AccpetEx函数一致
						(LPSOCKADDR*)&LocalAddr,				//5、用来返回连接的本地地址
						&localLen,								//6、用来返回本地地址的长度
						(LPSOCKADDR*)&ClientAddr,				//7、用来返回远程地址
						&remoteLen);							//8、用来返回远程地址的长度


		//设置更新属性
		setsockopt(acc->m_Socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenfd, sizeof(listenfd));

		//设置发送接收缓冲区
		int rece = __ServerInfo->ReceOne;
		int send = __ServerInfo->SendOne;
		errorCode = setsockopt(acc->m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)&send, sizeof(send));		//视频中无code =!!!!!!!!!!!!!!!!!
		errorCode = setsockopt(acc->m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)&rece, sizeof(rece));		//视频中无code =!!!!!!!!!!!

		//设置心跳包检查参数
		if (this->setHeartCheck(acc->m_Socket) < 0) return -2;

		//将用于和客户端通信的SOCKET绑定到完成端口中
		HANDLE handle = CreateIoCompletionPort((HANDLE)acc->m_Socket, m_Completeport, (DWORD)func::SC_WAIT_ACCEPT, 0);
		if (handle == NULL) return -3;

		//直接对位 性能最高
		S_CLIENT_BASE_INDEX* cindex = getClientIndex(acc->m_Socket);
		if (cindex == nullptr) return -4;

		//这里 我们就需要用到用户玩家数据了
		S_CLIENT_BASE* c = getFreeLinker();
		if (c == nullptr)
		{
			LOG_MSG("server full...\n");
			return -5;
		}

		//设置索引
		cindex->index = c->ID;

		//填充玩家新连接用户数据
		memcpy(c->ip, inet_ntoa(ClientAddr->sin_addr), MAX_IP_LEN);
		c->socketfd = acc->m_Socket;
		c->port = ntohs(ClientAddr->sin_port);
		c->state = func::S_Connect;
		c->time_Connet = (int)time(NULL);
		c->time_Heart = (int)time(NULL);

		//LOG_MSG("new connect...%s-%d \n", c->ip, c->port);

		//投递接受数据
		int ret = this->postRecv(acc->m_Socket);
		if (ret != 0)
		{
			//deleteConnectIP(c->ip);			课程无！！！！！！！！！！！！
			c->Reset();
			return -6;
		}

		//生成随机种子 发送一个安全码给客户端  随机加密码
		srand(time(NULL));
		u8 rcode = rand() % 125 + 1;
		this->begin(c->ID, CMD_RCODE);
		this->sss(c->ID, rcode);
		this->end(c->ID);
		c->rCode = rcode;

		updateConnect(true);

		if (onAcceptEvent != nullptr) this->onAcceptEvent(this, c, 0);
		AcceptContext::push(acc);
		this->postAccept();

		return 0;
	}

	//投递接收数据
	s32 TcpServer::postRecv(SOCKET s)
	{
		RecvContext* context = RecvContext::pop();
		context->m_Socket = s;

		unsigned long bytes = 0;
		unsigned long flag = 0;

		int err = WSARecv(context->m_Socket,     //1、 操作的套接字
			&context->m_wsaBuf,					 //2、 接收缓冲区
			1,									 //3、 wsaBuf数组中WSABUF结构的数目
			&bytes,								 //4、 如果接收操作立即完成,返回函数调用所接收到的字节数
			&flag,								 //5、 用来控制套接字的行为 一般设置为0
			&(context->m_OverLapped),			 //6、 重叠结构
			NULL);								 //7、 一个指向接收操作结束后调用的例程的指针

		if (SOCKET_ERROR == err)
		{
			int error = WSAGetLastError();
			if (error != WSA_IO_PENDING)
			{
				RecvContext::push(context);
				return -1;
			}
		}
		return 0;
	}

	//recv 接收数据
	s32 TcpServer::onRecv(void* context, s32 recvBytes, u32 tid)
	{
		RecvContext* rece = (RecvContext*)context;
		if (rece == NULL) return -1;

		S_CLIENT_BASE* c = client(rece->m_Socket, true);
		if (c == nullptr)
		{
			shutDown(rece->m_Socket, 0, NULL, 1001);
			RecvContext::push(rece);
			return -1;
		}

		c->threadID = tid;
		//保存数据
		s32 errorcode = onRecv_SaveData(c, rece->m_wsaBuf.buf, recvBytes);
		if (errorcode != 0)
		{
			c->is_RecvCompleted = true;
			shutDown(rece->m_Socket, 0, NULL, 1002);
			RecvContext::push(rece);
			return -2;
		}

		//继续投递接受数据
		int ret = this->postRecv(rece->m_Socket);
		if (ret != 0)
		{
			c->is_RecvCompleted = true;
			shutDown(rece->m_Socket, 0, c, 1003);
			RecvContext::push(rece);
			return -2;
		}
		c->is_RecvCompleted = true;
		RecvContext::push(rece);
		return 0;
	}

	//生产者--保存数据
	s32 TcpServer::onRecv_SaveData(S_CLIENT_BASE* c, char* buf, s32 recvBytes)
	{
		if (buf == nullptr) return -1;

		if (c->recv_Head == c->recv_Tail)
		{
			c->recv_Tail = 0;
			c->recv_Head = 0;
		}

		//buff缓冲区已满
		if (c->recv_Tail + recvBytes > __ServerInfo->ReceMax) return -2;

		memcpy(&c->recvBuf[c->recv_Tail], buf, recvBytes);
		c->recv_Tail += recvBytes;
		//c->is_RecvCompleted = true;			课程无！！！！！！！！！！！！！！！！！
		return 0;
	}

	//消费者 发送异步文件传送请求
	s32 TcpServer::postSend(S_CLIENT_BASE* c)
	{
		if (c->is_SendCompleted == false) return -1;
		if (c->ID < 0 ||
			c->state == S_SOCKET_STATE::S_Free ||
			c->closeState == func::S_CLOSE_SHUTDOWN ||
			c->socketfd == INVALID_SOCKET) return -2;

		if (c->send_Tail <= c->send_Head) return -3;

		s32 sendBytes = c->send_Tail - c->send_Head;
		if (sendBytes <= 0) return -4;
		if (sendBytes > __ServerInfo->SendOne) sendBytes = __ServerInfo->SendOne;
		//c->is_SendCompleted = false;         课程无！！！！！！！！！！！！！！！！！！！！

		SendContext* context = SendContext::pop();
		context->setSend(c->socketfd, &c->sendBuf[c->send_Head], sendBytes);

		unsigned long dwBytes = 0;
		unsigned long err = WSASend(context->m_Socket,
									&context->m_wsaBuf,
									1,
									&dwBytes,
									0,
									&(context->m_OverLapped),
									NULL);

		if (err == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error != WSA_IO_PENDING)
			{
				shutDown(context->m_Socket, 0, c, 1004);
				SendContext::push(context);
				return -5;
			}
		}
		return 0;
	}

	//消费者 发送数据成功
	s32 TcpServer::onSend(void* context, s32 sendBytes)
	{
		SendContext* sc = (SendContext*)context;
		if (sc == NULL) return -1;

		if (sc->m_wsaBuf.len != sendBytes)
		{
			shutDown(sc->m_Socket, 0, NULL, 1005);
			SendContext::push(sc);
			return -1;
		}
		//获取玩家数据
		S_CLIENT_BASE* c = client(sc->m_Socket, true);
		if (c == nullptr)
		{
			shutDown(sc->m_Socket, 0, NULL, 1006);
			SendContext::push(sc);
			return -1;
		}

		if (c->ID < 0 ||
			c->state == S_SOCKET_STATE::S_Free ||
			c->closeState == func::S_CLOSE_SHUTDOWN ||
			c->socketfd == INVALID_SOCKET)
		{
			c->is_SendCompleted = true;
			SendContext::push(sc);
			return -2;
		}

		//发送成功
		c->send_Head += sendBytes;
		c->is_SendCompleted = true;				
		SendContext::push(sc);
		return 0;
	}

	//**************************************************************
	//关闭socket
	s32 TcpServer::closeSocket(SOCKET socketfd, S_CLIENT_BASE* c, int kind)
	{
		if (socketfd == SOCKET_ERROR || socketfd == INVALID_SOCKET || socketfd == NULL) return -1;

		if (c != nullptr)
		{
			if (c->state == func::S_Free) return -1;
			if (c->state >= func::S_ConnectSecure)
			{
				this->updateSecurityConnect(false);
			}
		}

		//更新下链接数量
		switch (kind)
		{
		case 1001:
		case 1002:
		case 3004:
			RELEASE_SOCKET(socketfd);
			break;
		default:
			this->updateConnect(false);
			shutdown(socketfd, SD_BOTH);
			RELEASE_SOCKET(socketfd);
			break;
		}
		
		if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, c, kind);	

		return 0;
	}


	//关闭读写端
	void net::TcpServer::shutDown(SOCKET socketfd, s32 mode, S_CLIENT_BASE* c, int kind)
	{
		if (c != nullptr)
		{
			if (c->state == func::S_Free) return;
			if (c->closeState == func::S_CLOSE_SHUTDOWN) return;

			c->shutdown_kind = kind;
			c->time_Close = (int)time(NULL);
			c->closeState = func::S_CLOSE_SHUTDOWN;

			shutdown(socketfd, SD_BOTH);
			//取消IO操作
			CancelIoEx((HANDLE)socket, nullptr);

			if (onExceptEvent != nullptr) this->onExceptEvent(this, c, kind);
			return;
		}

		auto c2 = client(socketfd, true);
		if (c2 == nullptr)
		{
			LOG_MSG("find socketfd is error:client2=null %d- kind:%d line:%d\n", (int)socketfd, kind, __LINE__);
			return;
		}

		if (c2->state == func::S_Free) return;
		if (c2->closeState == func::S_CLOSE_SHUTDOWN) return;

		switch (mode)
		{
		case func::SC_WAIT_RECV:
			c2->is_RecvCompleted = true;
			break;
		case func::SC_WAIT_SEND:
			c2->is_SendCompleted = true;
			break;
		}

		c2->shutdown_kind = kind;
		c2->time_Close = (s32)time(NULL);
		c2->closeState = func::S_CLOSE_SHUTDOWN;
		shutdown(socketfd, SD_BOTH);

		if (onExceptEvent != nullptr) this->onExceptEvent(this, c2, kind);	
	}





}

#endif