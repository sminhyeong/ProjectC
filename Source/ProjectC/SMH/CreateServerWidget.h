// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateServerWidget.generated.h"


// 전방 선언
class ANetworkManager;
class UEditableTextBox;
class UButton;
class UTextBlock;
class USpinBox;
class UCheckBox;

/**
 * 
 */
 // 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnServerCreated,
    bool, bSuccess,          
    int32, ServerID,         
    const FString&, Message
);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateCancelled);

UCLASS()
class PROJECTC_API UCreateServerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // 위젯 바인딩
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> ServerNameTextBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> ServerPasswordTextBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> ServerIPTextBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USpinBox> ServerPortSpinBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USpinBox> MaxPlayersSpinBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCheckBox> HasPasswordCheckBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> CreateButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> CancelButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> StatusTextBlock;

    UPROPERTY(BlueprintAssignable)
    FOnServerCreated OnServerCreated;

    UPROPERTY(BlueprintAssignable)
    FOnCreateCancelled OnCreateCancelled;

    // 공개 함수들
    UFUNCTION(BlueprintCallable)
    void SetNetworkManager(ANetworkManager* NetworkMgr);

    UFUNCTION(BlueprintCallable)
    void SetUserID(int32 InUserID);

    UFUNCTION(BlueprintCallable)
    void ResetForm();

protected:
    // 버튼 이벤트
    UFUNCTION()
    void OnCreateButtonClicked();

    UFUNCTION()
    void OnCancelButtonClicked();

    UFUNCTION()
    void OnHasPasswordChanged(bool bIsChecked);

    // 입력 검증
    bool ValidateInput();
    void UpdateStatusText(const FString& Message, bool bIsError = false);

private:
    UPROPERTY()
    TObjectPtr<ANetworkManager> NetworkManager;

    int32 CurrentUserID;	
};
