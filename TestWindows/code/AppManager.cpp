#include "AppManager.h"
#include "AppGlobal.h"
#include <thread>
#include "../../share/ShareFunction.h"
#include "AppTest.h"


namespace app
{

	AppManager* __AppManager = nullptr;
	int temp_time = 0;
	char printfstr[1000];

	AppManager::AppManager()
	{
	}
	AppManager::~AppManager()
	{
	}

	//打印信息
	void printInfo()
	{
#ifdef  ____WIN32_
		int tempTime = (int)time(NULL) - temp_time;
		if (tempTime < 1) return;					//打印时间设置！！！！！！！！！！！！！！！！

		int concount = 0;
		int securitycount = 0;
		__TcpServer->getSecurityCount(concount, securitycount);


		sprintf_s(printfstr, "connect-%d  security-%d", concount, securitycount);
		SetWindowTextA(GetConsoleWindow(), printfstr);
#endif
	}


	void onUpdate()
	{
		if (__TcpServer == nullptr) return;

		__TcpServer->parseCommand();
		printInfo();

	}

	void AppManager::init()
	{
		share::InitData();
		//创建服务器 启动
		__TcpServer = net::NewTcpServer();

		__TcpServer->setOnClientAccept(onClientAccept);
		__TcpServer->setOnClientSecureConnect(onClientSecureConnect);
		__TcpServer->setOnClientDisconnect(onClientDisconnect);
		__TcpServer->setOnClientTimeout(onClientTimeout);
		__TcpServer->setOnClientExcept(onClientExcept);
		__TcpServer->runServer(5);						//开辟线程数量,可以自由更改!!!!!!!!!!!!!!!!!!!!!!!!!!!!


		__AppTest = new AppTest();
		__TcpServer->registerCommand(1000, __AppTest);


		//Sleep(5000);				//测试停止服务器
		//__TcpServer->stopServer();	//测试停止服务器
		while (true)
		{
			onUpdate();
			Sleep(10);
		}
	}

	int run()
	{
		if (__AppManager == nullptr)
		{
			__AppManager = new AppManager();
			__AppManager->init();
		}

		return 0;
	}





}