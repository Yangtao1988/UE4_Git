#include "AppTest.h"
#include "../../share/ShareFunction.h"
#include <thread>

#ifndef ____WIN32_
#include <sys/timeb.h>
#endif



namespace app
{

	IContainer* __AppTest = nullptr;

	//字节对齐
#pragma pack(push,packing)
#pragma pack(1)
	struct  testdata
	{
		u32   curtime;
		s32   job;
		u8    aa[100];
	};

#pragma pack(pop, packing)


	int temptime = 0;
	testdata ttdata;


	/*void ontestdata(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		testdata  ttdata;

		s32 index = 0;
		ts->read(c->ID, index);
		ts->read(c->ID, &ttdata, sizeof(testdata));

		LOG_MSG("AppTest data:%d-- id:%d/job:%d  arr:%d/%d/%d  \n",
				index, ttdata.curtime,ttdata.job,ttdata.aa[0],ttdata.aa[33],ttdata.aa[99]);


		ts->begin(c->ID, 1000);
		ts->sss(c->ID, index);
		ts->sss(c->ID, &ttdata, sizeof(testdata));
		ts->end(c->ID);

	}*/


	AppTest::AppTest()
	{
	}

	AppTest::~AppTest()
	{
	}

	void AppTest::onInit()
	{
	}

	void AppTest::onUpdate()
	{
		s32 tempTime = (s32)time(NULL) - temptime;
		if (tempTime < 1) return;					//打印数据事件！！！可更改
		temptime = (s32)time(NULL);

		//char name[20];
		//char key[20];
		//sprintf(name, "aaaa111");
		//sprintf(key, "sss11122");

		////模拟账号 登录游戏服务器
		//auto c = __TcpGame[0];
		//c->begin(1000);
		//c->sss(name,20);
		//c->sss(key,20);
		//c->end();
		//return;

		for (int i = 0; i < TESTCONNECT; i++)
		{
			auto c = __TcpGame[i];
			if (c->getData()->state < func::C_ConnectSecure) continue;

			memset(&ttdata, 0, sizeof(testdata));
#ifdef ____WIN32_
			ttdata.curtime = clock();
#else
			struct timeb tp;
			ftime(&tp);
			ttdata.curtime = tp.time;
			ttdata.job = tp.millitm;
#endif
			ttdata.aa[0] = 99;
			ttdata.aa[33] = 122;
			ttdata.aa[99] = 250;

			c->begin(1000);
			c->sss(i);
			c->sss(&ttdata, sizeof(testdata));
			c->end();
		}
	}


	void ontest(net::ITcpClient* tc)
	{
		s32 index = 0;
		tc->read(index);
		tc->read(&ttdata, sizeof(testdata));

		if (index == 0)
		{

#ifdef ____WIN32_
			int ftime = clock() - ttdata.curtime;
			LOG_MSG("AppTest: %d-%d-%d  ftime:%d  \n",
				ttdata.aa[0], ttdata.aa[33], ttdata.aa[99], ftime);
#else
			struct timeb tp;
			ftime(&tp);
			s32 fftime = (ttdata.curtime - tp.time) * 1000 + ttdata.job - tp.millitm;
			LOG_MSG("AppTest: ftime:%d \n", fftime);
#endif

		}
	}



	bool AppTest::onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		return true;
	}



	bool AppTest::onClientCommand(net::ITcpClient* tc, const u16 cmd)
	{
		auto c = tc->getData();
		if (c->state < func::C_ConnectSecure) return false;

		switch (cmd)
		{
		case 1000:
			ontest(tc);
			break;
		}

		return true;
	}




}