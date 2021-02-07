// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"


UCLASS()
class UE4SC_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,Category = "MyEngine")
	int AppInitGameInstance();
	
	UFUNCTION(BlueprintImplementableEvent,Category = "MyEngine")
    void onCommand(int cmd);
	UFUNCTION(BlueprintImplementableEvent,Category = "MyEngine")
    void onConnect(int errcode);
	UFUNCTION(BlueprintImplementableEvent,Category = "MyEngine")
    void onSecurity(int errcode);
	UFUNCTION(BlueprintImplementableEvent,Category = "MyEngine")
    void onDisconnect(int errcode);
	UFUNCTION(BlueprintImplementableEvent,Category = "MyEngine")
    void onExcept(int errcode);
	
	int32 GetTimeSeconds();
	virtual void Shutdown();
	
};

extern UMyGameInstance* __AppGameInstance;