#include "BlueprintFunction.h"
#include "AppManager.h"


int32 UBlueprintFunction::AppUpdate()
{
	app::onUpdate();
	return 0;
}

int32 UBlueprintFunction::disConnect()
{
	if(app::__TcpClient == nullptr) return -1;
	app::__TcpClient->disconnectServer(6000,"blue close");
	return 0;//课程无，自己新增的！！！！！！！！！！
}

bool UBlueprintFunction::isSecurity()
{
	if(app::__TcpClient == nullptr) return false;
	uint8 state = app::__TcpClient->getData()->state;
	if(state < func::C_ConnectSecure) return false;
	return true;
}

int32 UBlueprintFunction::read_int8()
{
	int8 temp = 0;
	app::__TcpClient->read(temp);
	return temp;
}

int32 UBlueprintFunction::read_int16()
{
	uint16 temp = 0;
	app::__TcpClient->read(temp);
	return temp;
}

int32 UBlueprintFunction::read_int32()
{
	int32 temp = 0;
	app::__TcpClient->read(temp);
	return temp;
}

float UBlueprintFunction::read_float()
{
	float temp = 0;
	app::__TcpClient->read(temp);
	return temp;
}

bool UBlueprintFunction::read_bool()
{
	bool temp = 0;
	app::__TcpClient->read(temp);
	return temp;
}

FVector UBlueprintFunction::read_FVector()
{
	FVector temp;
	app::__TcpClient->read(&temp,sizeof(FVector));
	return temp;
}

FRotator UBlueprintFunction::read_FRotator()
{
	FRotator temp;
	app::__TcpClient->read(&temp,sizeof(FRotator));
	return temp;
}

FString UBlueprintFunction::read_FString()
{
	int len = 0;
	app::__TcpClient->read(len);
	if (len < 1 || len > 1024 * 1024 * 5)
	{
		return "";
	}
	uint8* cc = (uint8*)FMemory::Malloc(len);
	app::__TcpClient->read(cc,len);
	FString value = UTF8_TO_TCHAR(cc);
	FMemory::Free(cc);
	return value;
}

FString UBlueprintFunction::read_FString_len(int32 len)
{
	uint8* cc = (uint8*)FMemory::Malloc(len);
	app::__TcpClient->read(cc,len);
	FString value = UTF8_TO_TCHAR(cc);
	FMemory::Free(cc);
	return value;
}

FPlayerBase UBlueprintFunction::read_PlayerBase()
{
	FPlayerBase data;
	//已知字节类型数据接收
	app::__TcpClient->read(&data,48);
	//未知字节数据接收
	data.nick = read_FString_len(20);
	return data;
}

void UBlueprintFunction::send_begin(int32 cmd)
{
	app::__TcpClient->begin(cmd);
}

void UBlueprintFunction::send_end(int32 cmd)
{
	app::__TcpClient->end();
}

void UBlueprintFunction::send_int8(int32 value)
{
	app::__TcpClient->sss((int8)value);
}

void UBlueprintFunction::send_int16(int32 value)
{
	app::__TcpClient->sss((int16)value);
}

void UBlueprintFunction::send_int32(int32 value)
{
	app::__TcpClient->sss(value);
}

void UBlueprintFunction::send_float(float value)
{
	app::__TcpClient->sss(value);
}

void UBlueprintFunction::send_bool(bool value)
{
	app::__TcpClient->sss(value);
}

void UBlueprintFunction::send_FVector(FVector value)
{
	app::__TcpClient->sss(&value,sizeof(FVector));
}

void UBlueprintFunction::send_FRotator(FRotator value)
{
	app::__TcpClient->sss(&value,sizeof(FRotator));

}

void UBlueprintFunction::send_FString(FString value)
{
	TCHAR* pdata = value.GetCharArray().GetData();
	uint8* cc = (uint8*)TCHAR_TO_UTF8(pdata);
	int size = value.GetCharArray().Num();
	app::__TcpClient->sss(size);
	app::__TcpClient->sss(cc,size);
}

void UBlueprintFunction::send_FString_len(FString value, int32 len)
{
	TCHAR* pdata = value.GetCharArray().GetData();
	uint8* cc = (uint8*)TCHAR_TO_UTF8(pdata);
	int size = value.GetCharArray().Num();
	if(size > len) size = len;
	app::__TcpClient->sss(cc,size);

	if(size == len) return;;
	int a = size - len;
	uint8* cc2 = (uint8*)FMemory::Malloc(a);
	app::__TcpClient->sss(cc2,a);

	FMemory::Free(cc2);
}
