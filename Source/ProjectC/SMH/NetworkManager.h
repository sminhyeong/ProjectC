#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkManager.generated.h"

// ���� ����
class FSocket;
class FInternetAddr;
class FTimerManager;
class UClientPacketManager;
struct FAccountLoginResponse;

// ���� ���� ������
UENUM(BlueprintType)
enum class ENetConnectionState : uint8
{
    Disconnected = 0,
    Connecting = 1,
    Connected = 2,
    Reconnecting = 3,
    Failed = 4
};

// ��������Ʈ ����
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
    // ���� ���� ���� �Լ���
    UFUNCTION(BlueprintCallable, Category = "Network")
    void ConnectToServer(const FString& ServerIP, int32 ServerPort);

    UFUNCTION(BlueprintCallable, Category = "Network")
    void DisconnectFromServer();

    UFUNCTION(BlueprintCallable, Category = "Network")
    bool IsConnectedToServer() const;

    UFUNCTION(BlueprintCallable, Category = "Network")
    ENetConnectionState GetConnectionState() const;

    // �α���/�α׾ƿ� ���� �Լ��� - ClientPacketManager ���
    UFUNCTION(BlueprintCallable, Category = "Network")
    bool LoginToServer(const FString& Username, const FString& Password, struct FAccountLoginResponse& OutResponse);

    UFUNCTION(BlueprintCallable, Category = "Network")
    bool LogoutFromServer(int32 UserID, FString& OutMessage);

    // ��������Ʈ
    UPROPERTY(BlueprintAssignable, Category = "Network")
    FOnConnectionStateChanged OnConnectionStateChanged;

protected:
    // ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Settings")
    FString DefaultServerIP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Settings")
    int32 DefaultServerPort;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Settings")
    float ReconnectInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Settings")
    float PacketTimeout;

private:
    // ���� ����
    FSocket* ClientSocket;
    FString CurrentServerIP;
    int32 CurrentServerPort;

    // ���� ����
    ENetConnectionState ConnectionState;

    // �翬�� Ÿ�̸�
    FTimerHandle ReconnectTimerHandle;

    // ��Ŷ �Ŵ���
    UPROPERTY()
    TObjectPtr<UClientPacketManager> ClientPacketManager;

    // ��Ŷ ���� ����
    TArray<uint8> ReceiveBuffer;
    static const int32 RECEIVE_BUFFER_SIZE = 4096;

    // ���� �Լ���
    void SetConnectionState(ENetConnectionState NewState);
    void StartReconnectTimer();
    void StopReconnectTimer();
    void TryReconnect();
    bool CreateSocket();
    void CleanupSocket();

    // ��Ŷ �ۼ��� ���� �Լ���
    bool SendPacketData(const TArray<uint8>& PacketData);
    bool ReceivePacketData(TArray<uint8>& OutPacketData, float TimeoutSeconds);

    // TODO: ���� ������ ��ɵ�
    // - ȸ������ ���
    // - ���� ���� ����/����Ʈ �ҷ�����
    // - ���� ������ ��� �ҷ�����
    // - �κ��丮 ���� �ҷ�����
    // - ������ ���� �ҷ�����
    // - �÷��̾� ���� �ҷ�����
};