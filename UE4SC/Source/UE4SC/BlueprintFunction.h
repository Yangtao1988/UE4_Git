#pragma once

#include "CoreMinimal.h"

#include "Chaos/AABBTree.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintFunction.generated.h"

#pragma pack(push,packing)
#pragma pack(1)

USTRUCT(BlueprintType)
struct FPlayerBase//玩家基础数据
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "YangEngine")
	int32  memid;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "YangEngine")
	int32  socketfd;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "YangEngine")
	int32  state;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "YangEngine")
	int32  curhp;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "YangEngine")
	int32  maxhp;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "YangEngine")
	float  speed;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "YangEngine")
	FVector   pos;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "YangEngine")
	FRotator   rot;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "YangEngine")
	FString  nick;
};
#pragma pack(pop, packing)


//接收、发送数据及更新
UCLASS()
class UE4SC_API UBlueprintFunction : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
	static int32 AppUpdate();
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static int32 disConnect();
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static bool isSecurity();
	//系统数据结构，解包
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static int32 read_int8();
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static int32 read_int16();
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static int32 read_int32();
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static float read_float();
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static bool read_bool();
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static FVector read_FVector();
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static FRotator read_FRotator();
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static FString read_FString();
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static FString read_FString_len(int32 len);
	
	//自定义数据结构体读写
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "YangEngine")
    static FPlayerBase read_PlayerBase();
	
	//系统数据结构,封包
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_begin(int32 cmd);
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_end(int32 cmd);
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_int8(int32 value);
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_int16(int32 value);
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_int32(int32 value);
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_float(float value);
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_bool(bool value);
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_FVector(FVector value);
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_FRotator(FRotator value);
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_FString(FString value);
	UFUNCTION(BlueprintCallable,Category = "YangEngine")
    static void send_FString_len(FString value,int32 len);
};
