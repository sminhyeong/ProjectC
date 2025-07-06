// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateServerWidget.h"
#include "NetworkManager.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/SpinBox.h"
#include "Components/CheckBox.h"

void UCreateServerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (CreateButton)
	{
		CreateButton->OnClicked.AddDynamic(this, &UCreateServerWidget::OnCreateButtonClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UCreateServerWidget::OnCancelButtonClicked);
	}

	if (HasPasswordCheckBox)
	{
		HasPasswordCheckBox->OnCheckStateChanged.AddDynamic(this, &UCreateServerWidget::OnHasPasswordChanged);
	}

	// 기본값 설정
	if (ServerIPTextBox)
	{
		ServerIPTextBox->SetText(FText::FromString(TEXT("127.0.0.1")));
	}

	if (ServerPortSpinBox)
	{
		ServerPortSpinBox->SetValue(7777);
		ServerPortSpinBox->SetMinValue(1024);
		ServerPortSpinBox->SetMaxValue(65535);
	}

	if (MaxPlayersSpinBox)
	{
		MaxPlayersSpinBox->SetValue(10);
		MaxPlayersSpinBox->SetMinValue(1);
		MaxPlayersSpinBox->SetMaxValue(100);
	}

	// 초기 상태
	NetworkManager = nullptr;
	CurrentUserID = 0;

	ResetForm();
}

void UCreateServerWidget::SetNetworkManager(ANetworkManager* NetworkMgr)
{
	NetworkManager = NetworkMgr;
}

void UCreateServerWidget::SetUserID(int32 InUserID)
{
	CurrentUserID = InUserID;
}

void UCreateServerWidget::ResetForm()
{
	if (ServerNameTextBox)
	{
		ServerNameTextBox->SetText(FText::GetEmpty());
	}

	if (ServerPasswordTextBox)
	{
		ServerPasswordTextBox->SetText(FText::GetEmpty());
		ServerPasswordTextBox->SetIsEnabled(false);
	}

	if (HasPasswordCheckBox)
	{
		HasPasswordCheckBox->SetIsChecked(false);
	}

	UpdateStatusText(TEXT("서버 정보를 입력해주세요."));
}

void UCreateServerWidget::OnCreateButtonClicked()
{
	if (!ValidateInput())
	{
		return;
	}

	if (!NetworkManager)
	{
		UpdateStatusText(TEXT("NetworkManager가 설정되지 않았습니다."), true);
		return;
	}

	if (CurrentUserID <= 0)
	{
		UpdateStatusText(TEXT("로그인이 필요합니다."), true);
		return;
	}

	// 입력값 수집
	FString ServerName = ServerNameTextBox->GetText().ToString();
	FString ServerPassword = HasPasswordCheckBox->IsChecked() ? ServerPasswordTextBox->GetText().ToString() : TEXT("");
	FString ServerIP = ServerIPTextBox->GetText().ToString();
	int32 ServerPort = static_cast<int32>(ServerPortSpinBox->GetValue());
	int32 MaxPlayers = static_cast<int32>(MaxPlayersSpinBox->GetValue());

	UpdateStatusText(TEXT("서버 생성 중..."));

	// 버튼 비활성화
	if (CreateButton)
	{
		CreateButton->SetIsEnabled(false);
	}

	// 서버 생성 요청
	int32 NewServerID = 0;
	FString CreateMessage;
	bool bSuccess = NetworkManager->CreateGameServer(CurrentUserID, ServerName, ServerPassword, ServerIP, ServerPort, MaxPlayers, NewServerID, CreateMessage);

	// 버튼 다시 활성화
	if (CreateButton)
	{
		CreateButton->SetIsEnabled(true);
	}

	if (bSuccess)
	{
		UpdateStatusText(FString::Printf(TEXT("서버 생성 성공! (ID: %d)"), NewServerID));

		// 폼 리셋
		ResetForm();
	}
	else
	{
		UpdateStatusText(FString::Printf(TEXT("서버 생성 실패: %s"), *CreateMessage), true);
	}
	OnServerCreated.Broadcast(bSuccess, NewServerID, CreateMessage);
}

void UCreateServerWidget::OnCancelButtonClicked()
{
	OnCreateCancelled.Broadcast();
}

void UCreateServerWidget::OnHasPasswordChanged(bool bIsChecked)
{
	if (ServerPasswordTextBox)
	{
		ServerPasswordTextBox->SetIsEnabled(bIsChecked);

		if (!bIsChecked)
		{
			ServerPasswordTextBox->SetText(FText::GetEmpty());
		}
	}
}

bool UCreateServerWidget::ValidateInput()
{
	// 서버 이름 검증
	if (!ServerNameTextBox || ServerNameTextBox->GetText().IsEmpty())
	{
		UpdateStatusText(TEXT("서버 이름을 입력해주세요."), true);
		return false;
	}

	FString ServerName = ServerNameTextBox->GetText().ToString();
	if (ServerName.Len() < 3 || ServerName.Len() > 20)
	{
		UpdateStatusText(TEXT("서버 이름은 3-20자 사이여야 합니다."), true);
		return false;
	}

	// 비밀번호 검증 (선택사항)
	if (HasPasswordCheckBox && HasPasswordCheckBox->IsChecked())
	{
		if (!ServerPasswordTextBox || ServerPasswordTextBox->GetText().IsEmpty())
		{
			UpdateStatusText(TEXT("비밀번호를 입력해주세요."), true);
			return false;
		}

		FString Password = ServerPasswordTextBox->GetText().ToString();
		if (Password.Len() < 4)
		{
			UpdateStatusText(TEXT("비밀번호는 최소 4자 이상이어야 합니다."), true);
			return false;
		}
	}

	// IP 주소 검증
	if (!ServerIPTextBox || ServerIPTextBox->GetText().IsEmpty())
	{
		UpdateStatusText(TEXT("서버 IP를 입력해주세요."), true);
		return false;
	}

	// 포트 검증
	if (!ServerPortSpinBox)
	{
		UpdateStatusText(TEXT("포트가 설정되지 않았습니다."), true);
		return false;
	}

	int32 Port = static_cast<int32>(ServerPortSpinBox->GetValue());
	if (Port < 1024 || Port > 65535)
	{
		UpdateStatusText(TEXT("포트는 1024-65535 사이여야 합니다."), true);
		return false;
	}

	return true;
}

void UCreateServerWidget::UpdateStatusText(const FString& Message, bool bIsError)
{
	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(FText::FromString(Message));

		// 에러일 때는 빨간색, 정상일 때는 흰색
		FLinearColor TextColor = bIsError ? FLinearColor::Red : FLinearColor::White;
		StatusTextBlock->SetColorAndOpacity(FSlateColor(TextColor));
	}

	UE_LOG(LogTemp, Log, TEXT("[CreateServerWidget] %s"), *Message);
}
