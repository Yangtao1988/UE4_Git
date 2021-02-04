#include "TcpServer.h"


#ifdef ____WIN32_

#include "IOPool.h"
using namespace func;

namespace net
{
	//运行线程
	void  TcpServer::runThread(int num)
	{
		m_IsRunning = true;
		m_ThreadNum = num;
		if (num > 10) m_ThreadNum = 10;

		for (int i = 0; i < m_ThreadNum; i++)
			m_workthread[i].reset(new std::thread(TcpServer::run, this, i));

		for (int i = 0; i < m_ThreadNum; i++)
			m_workthread[i]->detach();
	}



	void pushContext(IOContext* context)
	{
		switch (context->m_Mode)
		{
		case func::SC_WAIT_ACCEPT:
			AcceptContext::push((AcceptContext*)context);
			break;
		case func::SC_WAIT_RECV:
			RecvContext::push((RecvContext*)context);
			break;
		case func::SC_WAIT_SEND:
			SendContext::push((SendContext*)context);
			break;
		}
	}

	//工作线程
	void TcpServer::run(TcpServer* tcp, int id)
	{
		LOG_MSG("run workthread...%d\n", id);
		ULONG_PTR    key = 1;				//完成端口绑定的字段
		OVERLAPPED* overlapped = nullptr;	//连入Socket的时候建立的那个重叠结构  
		DWORD        recvBytes = 0;			//操作完成返回字节数

		while (tcp->m_IsRunning)
		{
			bool iscomplete = GetQueuedCompletionStatus(tcp->getCompletePort(), &recvBytes, &key, &overlapped, INFINITE);
			//参数1：完成端口内核对象句柄  参数2：发送或者接手字节数  参数3：自定义的数据结构指针  参数4：重叠数据结构  参数5：超时时间

			IOContext* context = CONTAINING_RECORD(overlapped, IOContext, m_OverLapped);
			//参数1：结构体中变量的地址  参数2：结构体原型  参数3：结构体的某个成员
			if (context == nullptr) continue;

			if (iscomplete == false)
			{
				DWORD dwErr = GetLastError();
				// 如果是超时了，就再继续等吧  
				if (WAIT_TIMEOUT == dwErr) continue;

				if (overlapped != NULL)
				{
					tcp->shutDown(context->m_Socket, context->m_Mode, NULL, 3001);
					pushContext(context);
					continue;
				}

				tcp->shutDown(context->m_Socket, context->m_Mode, NULL, 3002);
				pushContext(context);
				continue;
			}
			else
			{
				if (overlapped == NULL)
				{
					LOG_MSG("overlapped == NULL \n");
					break;
				}
				if (key != 0)
				{
					LOG_MSG("key != 0 \n");
					continue;
				}

				// 判断是否有客户端断开了
				if ((recvBytes == 0) && (context->m_Mode == func::SC_WAIT_RECV || context->m_Mode == func::SC_WAIT_SEND))
				{
					tcp->shutDown(context->m_Socket, context->m_Mode, NULL, 3003);
					pushContext(context);
					continue;
				}

				switch (context->m_Mode)
				{
				case func::SC_WAIT_ACCEPT:
				{
					auto acc = (AcceptContext*)context;
					int err = tcp->onAccept(acc);
					if (err != 0)
					{
						tcp->closeSocket(acc->m_Socket, NULL, 3004);
						AcceptContext::push(acc);
						tcp->postAccept();
					}
				}
				break;

				case func::SC_WAIT_RECV:
					tcp->onRecv(context, (int)recvBytes, id);
					break;

				case func::SC_WAIT_SEND:
					tcp->onSend(context, (int)recvBytes);
					break;
				}
			}
		}
		LOG_MSG("exit workthread...%d\n", id);
	}



}


#endif