// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstanceBase.h"
#include "NetworkManager.h"

// C++ GameInstance
void UNetGameInstanceBase::Init()
{
    Super::Init();
    NetworkManager = NewObject<UNetworkManager>(this);  // 여기서 생성
    NetworkManager->InitClientPacketManager();
}

void UNetGameInstanceBase::Shutdown()
{
    if (NetworkManager)
    {
        NetworkManager->DisconnectFromServer();
    }
    Super::Shutdown();
}