// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NetGameInstanceBase.generated.h"

class UNetworkManager;
/**
 * 
 */
UCLASS()
class PROJECTC_API UNetGameInstanceBase : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	// 블루프린트에서 접근할 수 있는 함수 추가
	UFUNCTION(BlueprintPure, Category = "Network")
	UNetworkManager* GetNetworkManager() const { return NetworkManager; }

private:
	UPROPERTY()
	TObjectPtr<UNetworkManager> NetworkManager;
};
