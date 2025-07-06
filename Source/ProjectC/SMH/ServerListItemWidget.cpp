#include "ServerListItemWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UServerListItemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 이벤트 바인딩
    if (ServerButton)
    {
        ServerButton->OnClicked.AddDynamic(this, &UServerListItemWidget::OnServerButtonClicked);
    }

    bIsMyServerItem = false;
}

void UServerListItemWidget::SetServerInfo(const FAccountGameServerInfo& ServerInfo)
{
    CurrentServerInfo = ServerInfo;

    // 서버 이름 설정
    if (ServerNameText)
    {
        ServerNameText->SetText(FText::FromString(ServerInfo.ServerName));
    }

    // 서버 소유자 설정
    if (OwnerNameText)
    {
        OwnerNameText->SetText(FText::FromString(FString::Printf(TEXT("방장: %s"), *ServerInfo.OwnerNickname)));
    }

    // 플레이어 수 설정
    if (PlayersText)
    {
        FString PlayersString = FString::Printf(TEXT("%d/%d"), ServerInfo.CurrentPlayers, ServerInfo.MaxPlayers);
        PlayersText->SetText(FText::FromString(PlayersString));

        // 서버가 가득 찬 경우 빨간색으로 표시
        if (ServerInfo.CurrentPlayers >= ServerInfo.MaxPlayers)
        {
            PlayersText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
        }
        else
        {
            PlayersText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        }
    }

    // 서버 IP:Port 설정
    if (ServerIPText)
    {
        FString IPPortString = FString::Printf(TEXT("%s:%d"), *ServerInfo.ServerIP, ServerInfo.ServerPort);
        ServerIPText->SetText(FText::FromString(IPPortString));
    }

    // 비밀번호 아이콘 표시/숨김
    if (PasswordIcon)
    {
        PasswordIcon->SetVisibility(ServerInfo.bHasPassword ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }

    // 서버가 가득 찬 경우 버튼 비활성화
    if (ServerButton)
    {
        bool bCanJoin = ServerInfo.CurrentPlayers < ServerInfo.MaxPlayers;
        ServerButton->SetIsEnabled(bCanJoin);

        // 서버 상태에 따른 색상 설정
        if (bIsMyServerItem)
        {
            // 내 서버 - 파란색
            ServerButton->SetColorAndOpacity(FLinearColor(0.3f, 0.6f, 1.0f, 1.0f));
        }
        else if (!bCanJoin)
        {
            // 가득 찬 서버 - 회색
            ServerButton->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
        }
        else
        {
            // 일반 서버 - 흰색
            ServerButton->SetColorAndOpacity(FLinearColor::White);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Server item updated: %s (%d/%d players)"),
        *ServerInfo.ServerName, ServerInfo.CurrentPlayers, ServerInfo.MaxPlayers);
}

void UServerListItemWidget::OnServerButtonClicked()
{
    // 서버가 가득 찬 경우 클릭 무시
    if (CurrentServerInfo.CurrentPlayers >= CurrentServerInfo.MaxPlayers)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot join server %s - Server is full"), *CurrentServerInfo.ServerName);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Server item clicked: %s (ID: %d)"),
        *CurrentServerInfo.ServerName, CurrentServerInfo.ServerID);

    OnItemClicked.Broadcast(CurrentServerInfo.ServerID, CurrentServerInfo.bHasPassword);
}