#include "ShareFunction.h"
#include "../engine/IDefine.h"
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"
#include "tinyxml/md5.h"

using namespace func;

namespace share
{

	int LoadServerXML(const char* filename)
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf(fpath, "%s%s", func::FileExePath, filename);

		if (func::__ServerInfo == nullptr)
		{
			func::__ServerInfo = new func::ConfigXML();
			memset(func::__ServerInfo, 0, sizeof(func::ConfigXML));
		}

		TiXmlDocument xml;
		if (!xml.LoadFile(fpath))
		{
			LOG_MSG("load config_server.xml iserror... \n");
			return -1;
		}


		TiXmlElement* xmlRoot = xml.RootElement();
		if (xmlRoot == NULL)
		{
			LOG_MSG("xmlRoot == NULL... \n");
			return -1;
		}

		//获取子节点信息1  
		TiXmlElement* xmlNode = xmlRoot->FirstChildElement("server");

		memcpy(__ServerInfo->SafeCode, xmlNode->Attribute("SafeCode"), 20);
		memcpy(__ServerInfo->Head, xmlNode->Attribute("Head"), 2);

		__ServerInfo->Port = atoi(xmlNode->Attribute("Port"));
		__ServerInfo->ID = atoi(xmlNode->Attribute("ID"));
		__ServerInfo->Type = 0;								//原来为1   ！！！！！！！！！！！！！！
		__ServerInfo->MaxPlayer = atoi(xmlNode->Attribute("MaxUser"));
		__ServerInfo->MaxConnect = atoi(xmlNode->Attribute("MaxConnect"));
		__ServerInfo->MaxAccpet = atoi(xmlNode->Attribute("MaxAccpet"));
		__ServerInfo->MaxRece = atoi(xmlNode->Attribute("MaxRece"));
		__ServerInfo->MaxSend = atoi(xmlNode->Attribute("MaxSend"));
		__ServerInfo->RCode = atoi(xmlNode->Attribute("CCode"));
		__ServerInfo->Version = atoi(xmlNode->Attribute("Version"));
		__ServerInfo->ReceOne = atoi(xmlNode->Attribute("ReceOne")) * 1024;
		__ServerInfo->ReceMax = atoi(xmlNode->Attribute("ReceMax")) * 1024;
		__ServerInfo->SendOne = atoi(xmlNode->Attribute("SendOne")) * 1024;
		__ServerInfo->SendMax = atoi(xmlNode->Attribute("SendMax")) * 1024;
		__ServerInfo->HeartTime = atoi(xmlNode->Attribute("HeartTime"));

		return 0;

	}


	int LoadClientXML(const char* filename)
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf(fpath, "%s/%s", FileExePath, filename);

		if (__ClientInfo == nullptr)
		{
			__ClientInfo = new ConfigXML();
			memset(__ClientInfo, 0, sizeof(ConfigXML));
		}

		TiXmlDocument xml;
		if (!xml.LoadFile(fpath))
		{
			LOG_MSG("load config_client.xml iserror... \n");
			return -1;
		}


		TiXmlElement* xmlRoot = xml.RootElement();
		if (xmlRoot == NULL)
		{
			LOG_MSG("xmlRoot == NULL... \n");
			return -1;
		}

		//获取子节点信息1  
		TiXmlElement* xmlNode = xmlRoot->FirstChildElement("client");

		memcpy(__ClientInfo->SafeCode, xmlNode->Attribute("SafeCode"), 20);
		memcpy(__ClientInfo->Head, xmlNode->Attribute("Head"), 2);

		__ClientInfo->MaxPlayer = atoi(xmlNode->Attribute("MaxUser"));
		__ClientInfo->MaxConnect = atoi(xmlNode->Attribute("MaxConnect"));
		__ClientInfo->RCode = atoi(xmlNode->Attribute("CCode"));
		__ClientInfo->Version = atoi(xmlNode->Attribute("Version"));
		__ClientInfo->ReceOne = atoi(xmlNode->Attribute("ReceOne")) * 1024;
		__ClientInfo->ReceMax = atoi(xmlNode->Attribute("ReceMax")) * 1024;
		__ClientInfo->SendOne = atoi(xmlNode->Attribute("SendOne")) * 1024;
		__ClientInfo->SendMax = atoi(xmlNode->Attribute("SendMax")) * 1024;
		__ClientInfo->HeartTime = atoi(xmlNode->Attribute("HeartTime"));
		__ClientInfo->AutoTime = atoi(xmlNode->Attribute("AutoTime"));

		//获取子节点信息1  
		xmlNode = xmlRoot->FirstChildElement("server");
		int num = atoi(xmlNode->Attribute("num"));
		char str[10];
		for (int i = 1; i <= num; i++)
		{
			memset(&str, 0, 10);
			sprintf(str, "client%d", i);
			xmlNode = xmlRoot->FirstChildElement(str);

			ServerListXML* serverlist = new ServerListXML();
			memcpy(serverlist->IP, xmlNode->Attribute("IP"), 16);
			serverlist->Port = atoi(xmlNode->Attribute("Port"));
			serverlist->ID = atoi(xmlNode->Attribute("ID"));

			__ServerListInfo.push_back(serverlist);
		}
		return 0;

	}



	bool share::InitData()
	{
		//设置函数指针
		func::MD5str = &utils::EncryptMD5str;

		//初始化数据
		func::InitData();


		//2、初始化XML
		int errs = LoadServerXML("config_server.xml");
		if (errs < 0) return false;

		errs = LoadClientXML("config_client.xml");
		if (errs < 0) return false;

		LOG_MSG("serverxml:%s-%d-%d\n", __ServerInfo->Head, __ServerInfo->ID, __ServerInfo->Port);
		LOG_MSG("clientxml:%s-%d-%d\n", __ClientInfo->Head, __ClientInfo->Version, __ClientInfo->RCode);

		for (int i = 0; i < __ServerListInfo.size(); i++)
		{
			LOG_MSG("clientxml:%s-%d-%d\n", __ServerListInfo[i]->IP, __ServerListInfo[i]->Port, __ServerListInfo[i]->ID);
		}
			
		return true;

	}


}

