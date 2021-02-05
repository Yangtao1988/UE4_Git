#ifndef  ____APPGLOBAL_H
#define  ____APPGLOBAL_H


#include "../../engine/INetBase.h"
#include "../../engine/IContainer.h"
#include <time.h>

namespace app
{
	extern net::ITcpServer* __TcpServer;

	//事件全局API
	extern void  onClientAccept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void  onClientSecureConnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void  onClientDisconnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void  onClientTimeout(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void  onClientExcept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
}



#endif

