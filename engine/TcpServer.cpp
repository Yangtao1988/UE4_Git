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


	//���з��������
	void TcpServer::runServer(s32 num)
	{
		//1�����������û�
		Linkers = new HashArray<S_CLIENT_BASE>(func::__ServerInfo->MaxConnect);
		for (int i = 0; i < Linkers->length; i++)
		{
			S_CLIENT_BASE* client = Linkers->Value(i);
			client->Init();
		}

		//2�������û�����
		LinkersIndexs = new HashArray<S_CLIENT_BASE_INDEX>(MAX_USER_SOCKETFD);
		for (int i = 0; i < LinkersIndexs->length; i++)
		{
			S_CLIENT_BASE_INDEX* client = LinkersIndexs->Value(i);
			client->Reset();
		}

		//3��ָ���ʼ��
		initCommands();

		//4����ʼ��socket
		s32 errcode = initSocket();
		if (errcode < 0)
		{
			LOG_MSG("InitServer err...%d \n", errcode);

			RELEASE_HANDLE(m_Completeport);
			RELEASE_SOCKET(listenfd);
			if (errcode != -2) WSACleanup();
			return;
		}

		//5����ʼ��Ͷ��
		initPost();
		//6�����ù����߳�
		this->runThread(num);
	}


	//ֹͣ������������������������������
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
		//1��������ɶ˿�
		m_Completeport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (m_Completeport == NULL) return -1;

		//2����ʼ��Windows Sockets DLL  �����׽���
		WSADATA  wsData;
		int errorcode = WSAStartup(MAKEWORD(2, 2), &wsData);
		if (errorcode != 0) return -2;

		//3�������׽��� �ص�IO WSA_FLAG_OVERLAPPED
		listenfd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (listenfd == INVALID_SOCKET) return -3;

		//4�������׽��ַ�����ģʽ �����׽ӿڵ�ģʽ��
		//������ֹ�׽ӿ�s�ķ�����ģʽ
		unsigned long ul = 1;
		errorcode = ioctlsocket(listenfd, FIONBIO, (unsigned long*)&ul);
		if (errorcode == SOCKET_ERROR) return -4;

		//5���رռ���socket�Ľ����뷢�ͻ�����
		int size = 0;
		setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(int));
		setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(int));

		//6���Ѽ���socket����ɶ˿ڰ
		HANDLE  handle = CreateIoCompletionPort((HANDLE)listenfd, m_Completeport, (DWORD)func::SC_WAIT_ACCEPT, 0);
		if (handle == nullptr) return -5;

		//7�����׽��� 
		//htons�ǽ����ͱ����������ֽ�˳��ת��������ֽ�˳��
		//���������ڵ�ַ�ռ�洢��ʽ��Ϊ��λ�ֽڴ�����ڴ�ĵ͵�ַ��
		sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(__ServerInfo->Port);
		serAddr.sin_addr.S_un.S_addr = INADDR_ANY;

		errorcode = ::bind(listenfd, (struct sockaddr*)&serAddr, sizeof(serAddr));
		if (errorcode == SOCKET_ERROR) return -6;


		//8�������׽��� �Ѿ�����������ֵĶ������� ������ϵͳ��ÿһ���˿����ļ������еĳ���,
		errorcode = listen(listenfd, SOMAXCONN);
		if (errorcode == SOCKET_ERROR) return -7;

		//9�� AcceptEx �� GetAcceptExSockaddrs ��GUID�����ڵ�������ָ��
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		GUID GuidGetAcceptEx = WSAID_GETACCEPTEXSOCKADDRS;
		DWORD dwBytes = 0;

		if (m_AcceptEx == nullptr)
		{
			//WSAIoctl()������һ���׽ӿڵ�ģʽ�����óɹ���WSAIoctl()��������0��
			//����Ļ���������SOCKET_ERROR����Ӧ�ó����ͨ��WSAGetLastError()����ȡ��Ӧ�Ĵ�����롣
			errorcode = WSAIoctl(listenfd,			//����Ҫ���Ƶ��׽ӿڵľ����
								SIO_GET_EXTENSION_FUNCTION_POINTER,	//�����еĲ����Ŀ��ƴ��룬��������Ҫ�ٿص�����
								&GuidAcceptEx,						//���뻺�����ĵ�ַ���������ָ��һ���������׽��ֽ��п��ƣ����������һ��guid��ָ��������ƺ�����guid����
								sizeof(GuidAcceptEx),				//���뻺�����Ĵ�С������Ϊguid�Ĵ�С����sizeof(&guid)����
								&m_AcceptEx,						//����������ĵ�ַ�����Ｔ�Ǻ���ָ��ĵ�ַ����
								sizeof(m_AcceptEx),					//����������Ĵ�С������ָ��Ĵ�С��
								&dwBytes,							//���ʵ���ֽ����ĵ�ַ
								NULL,								//WSAOVERLAPPED�ṹ�ĵ�ַ��һ��ΪNULL����
								NULL);								//һ��ָ�������������õ�����ָ�루һ��ΪNULL����
		}

		if (m_AcceptEx == nullptr || errorcode == SOCKET_ERROR) return -8;

		// ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
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


	//��ʼ��Ͷ��
	void TcpServer::initPost()
	{
		//Ͷ������
		for (int i = 0; i < __ServerInfo->MaxAccpet; i++)
			postAccept();
	}


	s32 TcpServer::postAccept()
	{
		//1������һ���µ�socketfd 
		SOCKET socketfd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (socketfd == INVALID_SOCKET) return -1;

		//2������Ϊ������ģʽ
		ULONG ul = 1;
		int errorCode = ioctlsocket(socketfd, FIONBIO, (unsigned long*)&ul);
		if (errorCode == SOCKET_ERROR)
		{
			closeSocket(socketfd, NULL, 1001);			
			return -2;
		}

		//3����ȡһ��������ճ����accept����
		AcceptContext* context = AcceptContext::pop();
		context->setSocket(listenfd, socketfd);

		//4��Ͷ��AcceptEx
		unsigned long dwBytes = 0;

		bool isaccept = m_AcceptEx(context->listenfd,       //1������soceket
									context->m_Socket,								//2��������socket
									context->m_buf,									//3�����ܻ����� a���ͻ��˷�����һ������ b��server��ַ c��client��ַ
									0,												//4��0����ȵ����ݵ���ֱ�ӷ��� ��0�ȴ�����
									sizeof(SOCKADDR_IN) + 16,						//5�����ص�ַ��С�����ȱ���Ϊ��ַ���� + 16�ֽ�
									sizeof(SOCKADDR_IN) + 16,						//6��Զ�˵�ַ��С�����ȱ���Ϊ��ַ���� + 16�ֽ�
									&dwBytes,										//7��ͬ����ʽ������ �������첽IOû�ã����ùܣ�
									&context->m_OverLapped);						//8�������ص�I / O��Ҫ�õ����ص��ṹ

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

		//��ȡ�ͻ������ݵ�ַ��Ϣ
		m_GetAcceptEx(acc->m_buf,								//1��ָ�򴫵ݸ�AcceptEx�������յ�һ�����ݵĻ�����
						0,										//2����������С������ʹ��ݸ�AccpetEx������һ��
						sizeof(SOCKADDR_IN) + 16,				//3�����ص�ַ��С������ʹ��ݸ�AccpetEx����һ��
						sizeof(SOCKADDR_IN) + 16,				//4��Զ�̵�ַ��С������ʹ��ݸ�AccpetEx����һ��
						(LPSOCKADDR*)&LocalAddr,				//5�������������ӵı��ص�ַ
						&localLen,								//6���������ر��ص�ַ�ĳ���
						(LPSOCKADDR*)&ClientAddr,				//7����������Զ�̵�ַ
						&remoteLen);							//8����������Զ�̵�ַ�ĳ���


		//���ø�������
		setsockopt(acc->m_Socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenfd, sizeof(listenfd));

		//���÷��ͽ��ջ�����
		int rece = __ServerInfo->ReceOne;
		int send = __ServerInfo->SendOne;
		errorCode = setsockopt(acc->m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)&send, sizeof(send));		//��Ƶ����code =!!!!!!!!!!!!!!!!!
		errorCode = setsockopt(acc->m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)&rece, sizeof(rece));		//��Ƶ����code =!!!!!!!!!!!

		//����������������
		if (this->setHeartCheck(acc->m_Socket) < 0) return -2;

		//�����ںͿͻ���ͨ�ŵ�SOCKET�󶨵���ɶ˿���
		HANDLE handle = CreateIoCompletionPort((HANDLE)acc->m_Socket, m_Completeport, (DWORD)func::SC_WAIT_ACCEPT, 0);
		if (handle == NULL) return -3;

		//ֱ�Ӷ�λ �������
		S_CLIENT_BASE_INDEX* cindex = getClientIndex(acc->m_Socket);
		if (cindex == nullptr) return -4;

		//���� ���Ǿ���Ҫ�õ��û����������
		S_CLIENT_BASE* c = getFreeLinker();
		if (c == nullptr)
		{
			LOG_MSG("server full...\n");
			return -5;
		}

		//��������
		cindex->index = c->ID;

		//�������������û�����
		memcpy(c->ip, inet_ntoa(ClientAddr->sin_addr), MAX_IP_LEN);
		c->socketfd = acc->m_Socket;
		c->port = ntohs(ClientAddr->sin_port);
		c->state = func::S_Connect;
		c->time_Connet = (int)time(NULL);
		c->time_Heart = (int)time(NULL);

		//LOG_MSG("new connect...%s-%d \n", c->ip, c->port);

		//Ͷ�ݽ�������
		int ret = this->postRecv(acc->m_Socket);
		if (ret != 0)
		{
			//deleteConnectIP(c->ip);			�γ��ޣ�����������������������
			c->Reset();
			return -6;
		}

		//����������� ����һ����ȫ����ͻ���  ���������
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

	//Ͷ�ݽ�������
	s32 TcpServer::postRecv(SOCKET s)
	{
		RecvContext* context = RecvContext::pop();
		context->m_Socket = s;

		unsigned long bytes = 0;
		unsigned long flag = 0;

		int err = WSARecv(context->m_Socket,     //1�� �������׽���
			&context->m_wsaBuf,					 //2�� ���ջ�����
			1,									 //3�� wsaBuf������WSABUF�ṹ����Ŀ
			&bytes,								 //4�� ������ղ����������,���غ������������յ����ֽ���
			&flag,								 //5�� ���������׽��ֵ���Ϊ һ������Ϊ0
			&(context->m_OverLapped),			 //6�� �ص��ṹ
			NULL);								 //7�� һ��ָ����ղ�����������õ����̵�ָ��

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

	//recv ��������
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
		//��������
		s32 errorcode = onRecv_SaveData(c, rece->m_wsaBuf.buf, recvBytes);
		if (errorcode != 0)
		{
			c->is_RecvCompleted = true;
			shutDown(rece->m_Socket, 0, NULL, 1002);
			RecvContext::push(rece);
			return -2;
		}

		//����Ͷ�ݽ�������
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

	//������--��������
	s32 TcpServer::onRecv_SaveData(S_CLIENT_BASE* c, char* buf, s32 recvBytes)
	{
		if (buf == nullptr) return -1;

		if (c->recv_Head == c->recv_Tail)
		{
			c->recv_Tail = 0;
			c->recv_Head = 0;
		}

		//buff����������
		if (c->recv_Tail + recvBytes > __ServerInfo->ReceMax) return -2;

		memcpy(&c->recvBuf[c->recv_Tail], buf, recvBytes);
		c->recv_Tail += recvBytes;
		//c->is_RecvCompleted = true;			�γ��ޣ���������������������������������
		return 0;
	}

	//������ �����첽�ļ���������
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
		//c->is_SendCompleted = false;         �γ��ޣ���������������������������������������

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

	//������ �������ݳɹ�
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
		//��ȡ�������
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

		//���ͳɹ�
		c->send_Head += sendBytes;
		c->is_SendCompleted = true;				
		SendContext::push(sc);
		return 0;
	}

	//**************************************************************
	//�ر�socket
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

		//��������������
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


	//�رն�д��
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
			//ȡ��IO����
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