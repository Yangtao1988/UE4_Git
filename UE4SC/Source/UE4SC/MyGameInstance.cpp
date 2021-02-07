#include "MyGameInstance.h"
#include "Engine/World.h"
#include "AppManager.h"

UMyGameInstance* __AppGameInstance = nullptr;

int UMyGameInstance::AppInitGameInstance()
{
	auto www = GetWorld();
	__AppGameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	
	//GetWorld()->WorldType
	app::run();
	
	return 0;
}

int32 UMyGameInstance::GetTimeSeconds()
{
	int32 ftime = GetWorld()->GetTimeSeconds() * 1000;
	return ftime;
}

void UMyGameInstance::Shutdown()
{
	if(app::__TcpClient != nullptr)
	{
		app::__TcpClient->setThread(true);
		app::__TcpClient->disconnectServer(8000,"shutdown");
	}
	Super::Shutdown();
}
