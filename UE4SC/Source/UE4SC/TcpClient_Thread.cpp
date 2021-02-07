#include "CoreMinimal.h"
#include "TcpClient.h"
#include "HAL/RunnableThread.h"
#include "Engine/Engine.h"

namespace net
{

	TcpClient_Thread::TcpClient_Thread(TcpClient* c)
	{
		this->tcp = c;
		thread = FRunnableThread::Create(this, TEXT("Yang thread..."), 0);

	}
	TcpClient_Thread:: ~TcpClient_Thread()
	{
		if (thread != nullptr)
		{
			delete thread;
			thread = nullptr;
		}
	}
	uint32 TcpClient_Thread::Run()
	{
		FString ss = FString::Printf(TEXT("client run thread..."));
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);
		tcp->run();
		return 0;
	}
	void TcpClient_Thread::Exit()
	{
		FString ss = FString::Printf(TEXT("client exit thread..."));
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);

	}
	void TcpClient_Thread::StopThread()
	{
		tcp->stop();
		if (thread != nullptr)
		{
			thread->WaitForCompletion();
		}
	}



	//************************************************************************************
	
	void TcpClient::stop()
	{
		isRuning = false;
	}


	void TcpClient::setThread(bool ispause)
	{
		isPause = ispause;
	}

	//线程
	void TcpClient::run()
	{
		while (isRuning)
		{
			if (!isPause)
			{
				switch (m_data.state)
				{
				case func::C_Free:
					{
						if (isFirstConnect == false)
						{
							m_data.reset();
							FPlatformProcess::Sleep(func::__ClientInfo->AutoTime);
							FString ss = FString::Printf(TEXT("auto connect..."));
							if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);
						}
						else
						{
							isFirstConnect = false;
						}
						this->connectServer();
					}
					break;
				default:
					this->onRecv();
					break;
				}
			}
			FPlatformProcess::Sleep(0.02);

		}
	}




}