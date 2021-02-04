#include "TcpServer.h"


#ifdef ____WIN32_

#include "IOPool.h"
using namespace func;

namespace net
{
	//�����߳�
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

	//�����߳�
	void TcpServer::run(TcpServer* tcp, int id)
	{
		LOG_MSG("run workthread...%d\n", id);
		ULONG_PTR    key = 1;				//��ɶ˿ڰ󶨵��ֶ�
		OVERLAPPED* overlapped = nullptr;	//����Socket��ʱ�������Ǹ��ص��ṹ  
		DWORD        recvBytes = 0;			//������ɷ����ֽ���

		while (tcp->m_IsRunning)
		{
			bool iscomplete = GetQueuedCompletionStatus(tcp->getCompletePort(), &recvBytes, &key, &overlapped, INFINITE);
			//����1����ɶ˿��ں˶�����  ����2�����ͻ��߽����ֽ���  ����3���Զ�������ݽṹָ��  ����4���ص����ݽṹ  ����5����ʱʱ��

			IOContext* context = CONTAINING_RECORD(overlapped, IOContext, m_OverLapped);
			//����1���ṹ���б����ĵ�ַ  ����2���ṹ��ԭ��  ����3���ṹ���ĳ����Ա
			if (context == nullptr) continue;

			if (iscomplete == false)
			{
				DWORD dwErr = GetLastError();
				// ����ǳ�ʱ�ˣ����ټ����Ȱ�  
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

				// �ж��Ƿ��пͻ��˶Ͽ���
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