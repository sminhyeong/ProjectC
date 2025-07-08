// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClientPacketManager.h"
#include "ServerListItemWidget.generated.h"

// 전방 선언
class UButton;
class UTextBlock;
class UImage;

// 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemClicked, int32, ServerID, bool, bHasPassword, int32, OwnerUserID);

UCLASS()
class PROJECTC_API UServerListItemWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // 위젯 바인딩
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ServerButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ServerNameText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> OwnerNameText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayersText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ServerIPText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> PasswordIcon;

    UPROPERTY(BlueprintAssignable)
    FOnItemClicked OnItemClicked;

    // 공개 함수들
    UFUNCTION(BlueprintCallable)
    void SetServerInfo(const FAccountGameServerInfo& ServerInfo);


protected:
    // 버튼 이벤트
    UFUNCTION()
    void OnServerButtonClicked();

private:
    FAccountGameServerInfo CurrentServerInfo;
    bool bIsMyServerItem;
};