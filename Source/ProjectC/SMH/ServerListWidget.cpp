#include "ServerListWidget.h"
#include "ServerListItemWidget.h"
#include "NetworkManager.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UServerListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 이벤트 바인딩
    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &UServerListWidget::OnRefreshButtonClicked);
    }

    if (CreateServerButton)
    {
        CreateServerButton->OnClicked.AddDynamic(this, &UServerListWidget::OnCreateServerButtonClicked);
    }

    // 초기 상태
    NetworkManager = nullptr;
    CurrentUserID = 0; 
    bShowingMyServers = false;

    UpdateStatusText(TEXT("서버 목록을 불러오려면 새로고침을 클릭하세요."));
}

void UServerListWidget::SetNetworkManager(UNetworkManager* NetworkMgr)
{
    NetworkManager = NetworkMgr;
    if(NetworkManager)
    CurrentUserID = NetworkManager->GetCurrentUserID();
}

void UServerListWidget::SetUserID(int32 InUserID)
{
   
}

void UServerListWidget::RefreshServerList()
{
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

    UpdateStatusText(TEXT("서버 목록을 불러오는 중..."));
    bShowingMyServers = false;

    // 기존 목록 클리어
    ClearServerList();

    // 버튼 비활성화
    if (RefreshButton)
    {
        RefreshButton->SetIsEnabled(false);
    }

    // 게임 서버 목록 요청
    TArray<FAccountGameServerInfo> ServerList;
    bool bSuccess = NetworkManager->GetGameServerList(ServerList);

    // 버튼 다시 활성화
    if (RefreshButton)
    {
        RefreshButton->SetIsEnabled(true);
    }

    if (bSuccess)
    {
        if (ServerList.Num() > 0)
        {
            PopulateServerList(ServerList);
            UpdateStatusText(FString::Printf(TEXT("%d개의 서버를 찾았습니다."), ServerList.Num()));
        }
        else
        {
            UpdateStatusText(TEXT("사용 가능한 서버가 없습니다."));
        }
    }
    else
    {
        UpdateStatusText(TEXT("서버 목록을 불러오는데 실패했습니다."), true);
    }
}

void UServerListWidget::ClearServerList()
{
    if (ServerScrollBox)
    {
        ServerScrollBox->ClearChildren();
    }
}

void UServerListWidget::OnRefreshButtonClicked()
{
    RefreshServerList();
}

void UServerListWidget::OnCreateServerButtonClicked()
{
    OnCreateServerRequested.Broadcast();
}

void UServerListWidget::OnServerItemClicked(int32 ServerID, bool bHasPassword, int32 OwnerUserID)
{
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

    UpdateStatusText(TEXT("서버에 접속 중..."));

    // 비밀번호 입력 필드에서 값 가져오기
    FString ServerPassword = TEXT("");
    if (ServerPasswordTextBox)
    {
        ServerPassword = ServerPasswordTextBox->GetText().ToString();
    }

    //// 비밀번호가 필요한 서버인데 입력하지 않은 경우
    //if (bHasPassword && ServerPassword.IsEmpty())
    //{
    //    UpdateStatusText(TEXT("비밀번호를 입력해주세요."), true);
    //    return;
    //}

    // 서버 접속 시도
    FString ServerIP;
    int32 ServerPort;
    FString JoinMessage;
    bool bSuccess = NetworkManager->JoinGameServer(CurrentUserID, ServerID, ServerPassword, ServerIP, ServerPort, JoinMessage);
    
    if (bSuccess)
    {
        UpdateStatusText(FString::Printf(TEXT("서버 접속 승인: %s:%d"), *ServerIP, ServerPort));
        OnServerSelected.Broadcast(ServerID, ServerIP, ServerPort, CurrentUserID == OwnerUserID);
        if (ServerPasswordTextBox)
        {
            ServerPasswordTextBox->SetText(FText::GetEmpty());
        }
    }
    else
    {
        UpdateStatusText(FString::Printf(TEXT("서버 접속 실패: %s"), *JoinMessage), true);
    }
}

void UServerListWidget::UpdateStatusText(const FString& Message, bool bIsError)
{
    if (StatusTextBlock)
    {
        StatusTextBlock->SetText(FText::FromString(Message));

        // 에러일 때는 빨간색, 정상일 때는 흰색
        FLinearColor TextColor = bIsError ? FLinearColor::Red : FLinearColor::White;
        StatusTextBlock->SetColorAndOpacity(FSlateColor(TextColor));
    }

    UE_LOG(LogTemp, Log, TEXT("[ServerListWidget] %s"), *Message);
}

void UServerListWidget::PopulateServerList(const TArray<FAccountGameServerInfo>& ServerList)
{
    if (!ServerScrollBox || !ServerItemWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("ServerScrollBox or ServerItemWidgetClass is null"));
        return;
    }

    // 기존 목록 클리어
    ClearServerList();

    // 서버 아이템 위젯 생성
    for (const FAccountGameServerInfo& ServerInfo : ServerList)
    {
        UServerListItemWidget* ItemWidget = CreateWidget<UServerListItemWidget>(this, ServerItemWidgetClass);
        if (ItemWidget)
        {
            // 서버 정보 설정
            ItemWidget->SetServerInfo(ServerInfo);

            // 클릭 이벤트 바인딩
            ItemWidget->OnItemClicked.AddDynamic(this, &UServerListWidget::OnServerItemClicked);

            // 스크롤박스에 추가
            ServerScrollBox->AddChild(ItemWidget);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Added %d server items to list"), ServerList.Num());
}