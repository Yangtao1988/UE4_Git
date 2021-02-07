#include "TcpClient.h"
#include "IDefine.h"

//#include "Framework/Text/IOS/IOSPlatformTextField.h"
#include "Interfaces/IPv4/IPv4Address.h"

namespace func
{
	ConfigXML* __ClientInfo = nullptr;

}

namespace net
{

	TcpClient::TcpClient()
	{
		isPause = false;
		isRuning = false;
		isFirstConnect = false;
		socketfd = nullptr;
		m_workthread = nullptr;

		onAcceptEvent = nullptr;
		onSecureEvent = nullptr;
		onDisconnectEvent = nullptr;
		onExceptEvent = nullptr;
		onCommand = nullptr;
	
	}

	TcpClient::~TcpClient()
	{
		if (m_workthread != nullptr)
		{
			delete m_workthread;
			m_workthread = nullptr;
		}
		if (socketfd != nullptr)
		{
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(socketfd);
			socketfd = nullptr;
		}
		m_data.reset();
	}

	//****************************************************************************************
	//初始化客户端数据，连接服务器
	void S_SERVER_BASE::init(int32 sid)
	{
		ID = 0;
		serverID = sid;
		serverType = 0;
		recvBuf = (uint8*)FMemory::Malloc(func::__ClientInfo->ReceMax);
		sendBuf = (uint8*)FMemory::Malloc(func::__ClientInfo->SendMax);
		recvBuf_Temp = (uint8*)FMemory::Malloc(func::__ClientInfo->ReceOne);
		ip = "127.0.0.1";
		port = 13550;
		reset();
	}

	void S_SERVER_BASE::reset()
	{
		state = 0;
		rCode = func::__ClientInfo->RCode;
		recv_Head = 0;
		recv_Tail = 0;
		recv_TempHead = 0;
		recv_TempTail = 0;
		is_Recved = false;

		send_Head = 0;
		send_Tail = 0;
		send_TempTail = 0;
		is_Sending = false;
		is_SendCompleted = false;
		time_Heart = 0;
		time_AutoConnect = 0;

		FMemory::Memset(recvBuf, 0, func::__ClientInfo->ReceMax);
		FMemory::Memset(sendBuf, 0, func::__ClientInfo->SendMax);
		FMemory::Memset(recvBuf_Temp, 0, func::__ClientInfo->ReceOne);
	}


	int32 TcpClient::initSocket()
	{
		if (socketfd != nullptr)
		{
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(socketfd);
			socketfd = nullptr;
		}
		
		socketfd = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream,TEXT("Yang"),false);
		return 0;
	}

	
	void TcpClient::runClient(int32 sid, FString ip, int32 port)
	{
		m_data.init(sid);
		m_data.time_AutoConnect = 0;
		m_data.ip = ip;
		m_data.port = port;
		
		isRuning = true;
		m_workthread = new TcpClient_Thread(this);
	}

	bool TcpClient::connectServer()
	{
		if(m_data.state >= func::C_Connect) return false;
		initSocket();
		if(socketfd == nullptr) return false;

		FIPv4Address ip;
		FIPv4Address::Parse(m_data.ip,ip);
		TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		addr->SetIp(ip.Value);
		addr->SetPort(m_data.port);			

		bool isconnect = socketfd->Connect(*addr);
		if (isconnect)
		{
			socketfd->SetNonBlocking();
			m_data.state = func::C_Connect;
			m_data.time_HeartTime = 0;
			//if (onAcceptEvent != nullptr) onAcceptEvent(this,0);
			return true;
		}
		return false;
	}

	void TcpClient::disconnectServer(const int32 errcode, FString err)
	{
		if(m_data.state == func::C_Free) return;
		if(socketfd == nullptr) return;

		socketfd->Close();
		m_data.reset();

		if(onDisconnectEvent != nullptr) onDisconnectEvent(this,errcode);//非课程注释
	}

	//********************************************************************************************
	//收发数据
	int32 TcpClient::onRecv()
	{
		if(socketfd == nullptr) return -1;
		FMemory::Memset(m_data.recvBuf_Temp,0,func::__ClientInfo->ReceOne);

		uint32 size;
		if(socketfd->HasPendingData(size) == false) return -1;

		int32 recvBytes = 0;
		bool isrecv = socketfd->Recv(m_data.recvBuf_Temp,func::__ClientInfo->ReceOne,recvBytes);
		if(isrecv && recvBytes > 0)
		{
			auto c = this->getData();
			if(c->recv_Tail == c->recv_Head)
			{
				c->recv_Tail = 0;
				c->recv_Head = 0;
			}

			if (c->recv_Tail + recvBytes >= func::__ClientInfo->ReceMax) return -1;
			FMemory::Memcpy(&c->recvBuf[c->recv_Tail],c->recvBuf_Temp,recvBytes);
			c->recv_Tail += recvBytes;
		}
		
		return 0;
	}

	
	int32 TcpClient::onSend()
	{
		auto c = this->getData();
		if (c->send_Tail <= c->send_Head) return 0;
		if(c->state < func::C_Connect) return -1;
		int32 sendlen = c->send_Tail - c->send_Head;
		if(sendlen < 1) return 0;

		int32 sendBytes = 0;
		bool issend = socketfd->Send(&c->sendBuf[c->send_Head],sendlen,sendBytes);
		if (issend && sendBytes > 0)
		{
			c->send_Head += sendBytes;
		}
		return 0;
	}
	
	
}