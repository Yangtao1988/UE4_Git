#ifndef  __APPMANAGER_H
#define  __APPMANAGER_H

#include "TcpClient.h"

namespace app
{

	class AppManager
	{
	public:
		AppManager();
		~AppManager();

		void  init();
	};
	extern void onUpdate();
	extern int run();
	extern net::TcpClient* __TcpClient;
}


#endif