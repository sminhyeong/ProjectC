#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClientPacketManager.h"
#include "ServerListWidget.generated.h"

// 전방 선언
class ANetworkManager;
class UServerListItemWidget;
class UScrollBox;
class UButton;
class UTextBlock;
class UEditableTextBox;

// 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnServerSelected, int32, ServerID, const FString&, ServerIP, int32, ServerPort, bool , IsOwnning);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateServerRequested);

UCLASS()
class PROJECTC_API UServerListWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // 위젯 바인딩
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UScrollBox> ServerScrollBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> RefreshButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> CreateServerButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> StatusTextBlock;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> ServerPasswordTextBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PasswordLabel;

    // 서버 아이템 위젯 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Classes")
    TSubclassOf<UServerListItemWidget> ServerItemWidgetClass;

    UPROPERTY(BlueprintAssignable)
    FOnServerSelected OnServerSelected;

    UPROPERTY(BlueprintAssignable)
    FOnCreateServerRequested OnCreateServerRequested;

    // 공개 함수들
    UFUNCTION(BlueprintCallable)
    void SetNetworkManager(ANetworkManager* NetworkMgr);

    UFUNCTION(BlueprintCallable)
    void SetUserID(int32 InUserID);

    UFUNCTION(BlueprintCallable)
    void RefreshServerList();

    UFUNCTION(BlueprintCallable)
    void ClearServerList();

protected:
    // 버튼 이벤트
    UFUNCTION()
    void OnRefreshButtonClicked();

    UFUNCTION()
    void OnCreateServerButtonClicked();

    // 서버 아이템 이벤트
    UFUNCTION()
    void OnServerItemClicked(int32 ServerID, bool bHasPassword, int32 OwnerUserIz);

    // UI 업데이트
    void UpdateStatusText(const FString& Message, bool bIsError = false);
    void PopulateServerList(const TArray<FAccountGameServerInfo>& ServerList);

private:
    UPROPERTY()
    TObjectPtr<ANetworkManager> NetworkManager;

    int32 CurrentUserID;
    bool bShowingMyServers;
};