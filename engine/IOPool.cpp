#include "IOPool.h"

#ifdef ____WIN32_

#include <concurrent_queue.h>

Concurrency::concurrent_queue<AcceptContext*> __accepts;
Concurrency::concurrent_queue<RecvContext*>   __recvs;
Concurrency::concurrent_queue<SendContext*>   __sends;


IOContext::IOContext()
{
	m_Socket = INVALID_SOCKET;
	m_Mode = 0;
	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}


IOContext::~IOContext()
{
}

//**************************************************************
//新的连接

AcceptContext::AcceptContext(int mode, SOCKET listfd, SOCKET sfd)
{
	m_Mode = mode;
	listenfd = listfd;
	m_Socket = sfd;

	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}


AcceptContext::~AcceptContext(void)
{
	m_Mode = 0;
	listenfd = NULL;
	m_Socket = NULL;

	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}


void AcceptContext::clear()
{
	listenfd = NULL;
	m_Socket = NULL;
	memset(&m_OverLapped, 0, sizeof(m_OverLapped));

}


void AcceptContext::setSocket(SOCKET listfd, SOCKET sfd)
{
	listenfd = listfd;
	m_Socket = sfd;
}


AcceptContext* AcceptContext::pop()
{
	AcceptContext* buff = nullptr;
	if (__accepts.empty() == true)
	{
		buff = new  AcceptContext(func::SC_WAIT_ACCEPT, NULL, NULL);
	}
	else
	{
		__accepts.try_pop(buff);
		if (buff == nullptr)
		{
			buff = new AcceptContext(func::SC_WAIT_ACCEPT, NULL, NULL);
		}
	}
	return buff;
}


void AcceptContext::push(AcceptContext* acc)
{
	if (acc == nullptr) return;
	if (__accepts.unsafe_size() > 1000)
	{
		delete acc;
		return;
	}
	acc->clear();
	__accepts.push(acc);
}


int AcceptContext::getCount()
{
	return __accepts.unsafe_size();
}

//**************************************************************
//接收数据缓存池

RecvContext::RecvContext(int mode)
{
	m_Socket = INVALID_SOCKET;
	m_Mode = mode;

	m_Buffs = new char[func::__ServerInfo->ReceOne];

	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));
	ZeroMemory(m_Buffs, func::__ServerInfo->ReceOne);

	m_wsaBuf.buf = m_Buffs;
	m_wsaBuf.len = func::__ServerInfo->ReceOne;
}


RecvContext::~RecvContext(void)
{
	m_Socket = INVALID_SOCKET;
	m_Mode = -1;
}


void RecvContext::clear()
{
	m_Socket = INVALID_SOCKET;

	ZeroMemory(m_Buffs, func::__ServerInfo->ReceOne);
	m_wsaBuf.buf = m_Buffs;
	m_wsaBuf.len = func::__ServerInfo->ReceOne;
}


RecvContext* RecvContext::pop()
{
	RecvContext* buff = nullptr;
	if (__recvs.empty() == true)
	{
		buff = new  RecvContext(func::SC_WAIT_RECV);
	}
	else
	{
		__recvs.try_pop(buff);
		if (buff == nullptr)
		{
			buff = new RecvContext(func::SC_WAIT_RECV);
		}
	}
	return buff;
}


void RecvContext::push(RecvContext* buff)
{
	if (buff == nullptr) return;
	if (__recvs.unsafe_size() > 1000)
	{
		delete buff;
		return;
	}
	buff->clear();
	__recvs.push(buff);
}


int RecvContext::getCount()
{
	return __recvs.unsafe_size();
}


//**************************************************************
//发送数据缓存池
SendContext::SendContext(int mode)
{
	m_Socket = INVALID_SOCKET;
	m_Mode = mode;
	m_Buffs = new char[func::__ServerInfo->SendOne];
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));
	ZeroMemory(m_Buffs, func::__ServerInfo->SendOne);
	m_wsaBuf.buf = m_Buffs;
	m_wsaBuf.len = func::__ServerInfo->SendOne;
}

SendContext::~SendContext(void)
{
	m_Socket = INVALID_SOCKET;
	m_Mode = -1;
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));
}

void SendContext::clear()
{
	m_Socket = INVALID_SOCKET;
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));
	ZeroMemory(m_Buffs, func::__ServerInfo->SendOne);
	m_wsaBuf.buf = m_Buffs;
	m_wsaBuf.len = func::__ServerInfo->SendOne;
}


int SendContext::setSend(SOCKET s, char* data, const int sendByte)
{
	m_Socket = s;
	if (&m_wsaBuf)
	{
		if (sendByte != 0 && data != NULL)
		{
			memcpy(m_Buffs, data, sendByte);
			m_wsaBuf.buf = m_Buffs;
			m_wsaBuf.len = sendByte;
		}
	}
	return sendByte;
}

SendContext* SendContext::pop()
{
	SendContext* buff = nullptr;
	if (__sends.empty() == true)
	{
		buff = new  SendContext(func::SC_WAIT_SEND);
	}
	else
	{
		__sends.try_pop(buff);
		if (buff == nullptr)
		{
			buff = new  SendContext(func::SC_WAIT_SEND);
		}
	}
	return buff;
}

void SendContext::push(SendContext* buff)
{
	if (buff == nullptr) return;
	if (__sends.unsafe_size() > 1000)
	{
		delete buff;
		return;
	}
	buff->clear();
	__sends.push(buff);
}


int SendContext::getCount()
{
	return __sends.unsafe_size();
}


#endif