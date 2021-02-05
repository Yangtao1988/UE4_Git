#include "AppTest.h"
#include "../../share/ShareFunction.h"

namespace app
{

	IContainer* __AppTest = nullptr;

	//×Ö½Ú¶ÔÆë
/*
#pragma pack(push,packing)
#pragma pack(1)
	struct  testdata
	{
		u32   curtime;
		s32   job;
		u8    aa[100];
	};

#pragma pack(pop, packing)


	void ontestdata(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		testdata  ttdata;

		s32 index = 0;
		ts->read(c->ID, index);
		ts->read(c->ID, &ttdata, sizeof(testdata));

		//LOG_MSG("AppTest data:%d-- id:%d/job:%d  arr:%d/%d/%d  \n",
				//index, ttdata.curtime,ttdata.job,ttdata.aa[0],ttdata.aa[33],ttdata.aa[99]);


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
	}

	bool AppTest::onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_ConnectSecure))
		{
			LOG_MSG("AppTest err...line:%d \n", __LINE__);
			return false;
		}

		/*switch (cmd)
		{
		case 1000:
			ontestdata(ts, c);
			break;
		}*/

		return true;			
	}


}