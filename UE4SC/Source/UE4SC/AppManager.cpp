#include "AppManager.h"
#include "MyGameInstance.h"


namespace app
{

	net::TcpClient* __TcpClient = nullptr;
	AppManager* __AppManager = nullptr;
	
	AppManager::AppManager()
	{
	}
	AppManager::~AppManager()
	{
	}

	void onUpdate()
	{
		if (__TcpClient == nullptr) return;
		__TcpClient->parseCommand();
	}


	void onCommand(net::TcpClient* tcp,const int32 code)
	{
		__AppGameInstance->onCommand(code);
	}
	
	void onConnect(net::TcpClient* tcp,const int32 code)
	{
		__AppGameInstance->onConnect(code);
	}

	void onSecurityConnect(net::TcpClient* tcp,const int32 code)
	{
		__AppGameInstance->onSecurity(code);
	}
	
	void onDisconnect(net::TcpClient* tcp,const int32 code)
	{
		__AppGameInstance->onDisconnect(code);
	}
	
	void onExcept(net::TcpClient* tcp,const int32 code)
	{
		__AppGameInstance->onExcept(code);
	}
	
	
	
	//XML信息，可更改
	void InitClientXML()
	{
		if(func::__ClientInfo == nullptr) func::__ClientInfo = new func::ConfigXML();

		func::__ClientInfo->SafeCode = "testcode";
		func::__ClientInfo->Head[0] = 'D';
		func::__ClientInfo->Head[1] = 'E';
		func::__ClientInfo->RCode = 130;
		func::__ClientInfo->Version = 20180408;
		func::__ClientInfo->ReceOne = 8 * 1024;
		func::__ClientInfo->ReceMax = 256 * 1024;
		func::__ClientInfo->SendOne = 8 * 1024;
		func::__ClientInfo->SendMax = 256 * 1024;
		func::__ClientInfo->HeartTime = 15;
		func::__ClientInfo->AutoTime = 3;

	}
	
	void AppManager::init()
	{
		InitClientXML();
		__TcpClient = new net::TcpClient();
		__TcpClient->setOnConnect(onConnect);
		__TcpClient->setOnConnectSecure(onSecurityConnect);
		__TcpClient->setOnDisconnect(onDisconnect);
		__TcpClient->setOnExcept(onExcept);
		__TcpClient->setOnCommand(onCommand);
		//运行客户端
		__TcpClient->runClient(0, "127.0.0.1", 13550);
		__TcpClient->getData()->ID = 0;
		
	}


	int run()
	{
		if (__AppManager == nullptr)
		{
			__AppManager = new AppManager();
			__AppManager->init();
		}
		__TcpClient->setThread(false);
		return 0;
	}

}
