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



	void onUpdate()
	{
		if (__AppTest == nullptr) return;

		for (u32 i = 0; i < TESTCONNECT; i++)
		{
			__TcpGame[i]->parseCommand();
		}

		__AppTest->onUpdate();

	}

	void AppManager::init()
	{
		bool isload = share::InitData();
		if (!isload) return;
		if (func::__ServerListInfo.size() < 1) return;		//���ӷ���������


		__AppTest = new AppTest();
		auto xml = func::__ServerListInfo[0];		//��ȡxml��һ���ļ�
		__TcpGame.reserve(TESTCONNECT);				//����ķ���������

		for (u32 i = 0; i < TESTCONNECT; i++)
		{
			auto client = net::NewTcpClient();
			client->setOnConnect(onConnect);
			client->setOnConnectSecure(onSecureConnect);
			client->setOnDisconnect(onDisconnect);
			client->setOnExcept(onExcept);
			__TcpGame.emplace_back(client);

			//���пͻ���
			client->runClient(xml->ID, xml->IP, xml->Port);
			client->getData()->ID = i;

			//�ͻ���ע��ָ��
			client->registerCommand(1000, __AppTest);			//����ָ��1000

		}

		//Sleep(5000);					//����ֹͣ������
		//__TcpServer->stopServer();	//����ֹͣ������
		while (true)
		{
			onUpdate();
#ifdef ____WIN32_
			Sleep(1);
#else
			usleep(2000);
#endif // ____WIN32_
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