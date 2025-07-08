#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClientPacketManager.h"
#include "NetworkManager.generated.h"

// 전방 선언
class FSocket;
class FInternetAddr;
class FTimerManager;
class UClientPacketManager;

// 연결 상태 열거형
UENUM(BlueprintType)
enum class ENetConnectionState : uint8
{
    Disconnected = 0,
    Connecting = 1,
    Connected = 2,
    Reconnecting = 3,
    Failed = 4
};

// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionStateChanged, ENetConnectionState, NewState);

UCLASS(BlueprintType, Blueprintable)
class PROJECTC_API ANetworkManager : public AActor
{
    GENERATED_BODY()

public:
    ANetworkManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Destroyed() override;

public:
    // 서버 연결 관련 함수들
    UFUNCTION(BlueprintCallable, Category = "Network")
    void ConnectToServer(const FString& ServerIP, int32 ServerPort);

    UFUNCTION(BlueprintCallable, Category = "Network")
    void DisconnectFromServer();

    UFUNCTION(BlueprintCallable, Category = "Network")
    bool IsConnectedToServer() const;

    UFUNCTION(BlueprintCallable, Category = "Network")
    ENetConnectionState GetConnectionState() const;

    // 로그인/로그아웃 관련 함수들 - ClientPacketManager 사용
    UFUNCTION(BlueprintCallable, Category = "Network")
    bool LoginToServer(const FString& Username, const FString& Password, FString& OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Network")
    bool LogoutFromServer(FString& OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Network")
    bool CreateAccountToServer(const FString& Username, const FString& Password, const FString& Nickname, int32& OutUserID, FString& OutMessage);
    
    // 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Network")
    FOnConnectionStateChanged OnConnectionStateChanged;

    UFUNCTION(BlueprintCallable, Category = "Network")
    bool CreateGameServer(int32 UserID, const FString& ServerName, const FString& ServerPassword, const FString& ServerIP, int32 ServerPort, int32 MaxPlayers, int32& OutServerID, FString& OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Network")
    bool GetGameServerList(TArray<struct FAccountGameServerInfo>& OutServerList);

    UFUNCTION(BlueprintCallable, Category = "Network")
    bool JoinGameServer(int32 UserID, int32 ServerID, const FString& ServerPassword, FString& OutServerIP, int32& OutServerPort, FString& OutMessage);

    //플레이어 데이터 조회
    UFUNCTION(BlueprintCallable, Category = "Network|Player Data")
    bool GetPlayerData(int32 UserID, struct FAccountPlayerData& OutPlayerData, FString& OutMessage);

    //플레이어 인벤토리 조회
    UFUNCTION(BlueprintCallable, Category = "Network|Inventory")
    bool GetPlayerInventory(int32 UserID, TArray<struct FAccountItemInfo>& OutInventoryItems, FString& OutMessage);

    //Store whghl
    UFUNCTION(BlueprintCallable, Category = "Network|Shop")
    bool GetStoreItemList(int32 ShopID, TArray<struct FAccountItemInfo>& OutShopItems, FString& OutMessage);

    //Store 거래 관련 함수들
    UFUNCTION(BlueprintCallable, Category = "Network|Shop")
    bool PurchaseItem(int32 UserID, int32 ItemID, int32 ItemCount, int32& OutNewGold, FString& OutMessage);

    UFUNCTION(BlueprintCallable, Category = "Network|Shop")
    bool SellItem(int32 UserID, int32 ItemID, int32 ItemCount, int32& OutNewGold, FString& OutMessage);

    //단일 아이템 정보 조회
    UFUNCTION(BlueprintCallable, Category = "Network|Item")
    bool GetSingleItemInfo(int32 UserID, int32 ItemID, FAccountItemInfo& OutItemInfo, FString& OutMessage);

    //인벤토리에서 아이템 제거
    UFUNCTION(BlueprintCallable, Category = "Network|Inventory")
    bool RemoveItemFromInventory(int32 UserID, int32 ItemID, int32 ItemCount, FString& OutMessage);

    //인벤토리의 아이템 갯수 추가
    UFUNCTION(BlueprintCallable, Category = "Network|Item")
    bool AddItemToInventory(int32 UserID, int32 ItemID, int32 ItemCount, FString& OutMessage);

    // 아이템 수량 직접 설정
    UFUNCTION(BlueprintCallable, Category = "Network|Item")
    bool SetItemQuantity(int32 UserID, int32 ItemID, int32 NewQuantity, FString& OutMessage);

    // 유저 데이터 접근 함수들
    UFUNCTION(BlueprintPure, Category = "User Data")
    bool IsLogin() const;

    UFUNCTION(BlueprintPure, Category = "User Data")
    int32 GetCurrentUserID() const;

    UFUNCTION(BlueprintPure, Category = "User Data")
    FString GetCurrentUsername() const;

    UFUNCTION(BlueprintPure, Category = "User Data")
    FString GetCurrentNickname() const;

    UFUNCTION(BlueprintPure, Category = "User Data")
    int32 GetCurrentLevel() const;

    UFUNCTION(BlueprintPure, Category = "User Data")
    FAccountLoginResponse GetCurrentUserData() const;

protected:
    // 서버 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Settings")
    FString DefaultServerIP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Settings")
    int32 DefaultServerPort;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Settings")
    float ReconnectInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Settings")
    float PacketTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Settings")
    float ConnectionTimeout = 10.0f; // 연결 타임아웃 설정

    //로그인한 유저의 데이터 저장
    FAccountLoginResponse CurrentUserData;

private:
    // 소켓 관련
    FSocket* ClientSocket;
    FString CurrentServerIP;
    int32 CurrentServerPort;

    // 연결 상태
    ENetConnectionState ConnectionState;

    // 재연결 타이머
    FTimerHandle ReconnectTimerHandle;

    // 패킷 매니저
    UPROPERTY()
    TObjectPtr<UClientPacketManager> ClientPacketManager;

    // 패킷 수신 관련
    TArray<uint8> ReceiveBuffer;
    static const int32 RECEIVE_BUFFER_SIZE = 4096;

    // 내부 함수들
    void SetConnectionState(ENetConnectionState NewState);
    void StartReconnectTimer();
    void StopReconnectTimer();
    void TryReconnect();
    bool CreateSocket();
    void CleanupSocket();

    // 패킷 송수신 관련 함수들
    bool SendPacketData(const TArray<uint8>& PacketData);
    bool ReceivePacketData(TArray<uint8>& OutPacketData, float TimeoutSeconds);


    bool CreateServerAddress(const FString& IP, int32 Port, TSharedRef<FInternetAddr>& OutAddress);
   
    // 유저 데이터 초기화 함수
    void ClearUserData();


    // TODO: 추후 구현할 기능들
    // - 회원가입 기능
    // - 게임 서버 생성/리스트 불러오기
    // - 상점 아이템 목록 불러오기
    // - 인벤토리 정보 불러오기
    // - 아이템 정보 불러오기
    // - 플레이어 정보 불러오기
};