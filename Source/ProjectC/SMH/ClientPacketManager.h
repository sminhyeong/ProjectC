// ClientPacketManager.h - ������ ��� ����
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "flatbuffers/flatbuffers.h"
#include "ClientPacketManager.generated.h"

// === UE5�� ����ü ���ǵ� ===

USTRUCT(BlueprintType)
struct PROJECTC_API FAccountLoginResponse
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Login Response")
    bool bSuccess = false;

    UPROPERTY(BlueprintReadWrite, Category = "Login Response")
    int32 UserID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Login Response")
    FString Username;

    UPROPERTY(BlueprintReadWrite, Category = "Login Response")
    FString Nickname;

    UPROPERTY(BlueprintReadWrite, Category = "Login Response")
    int32 Level = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Login Response")
    FString Message;
};

USTRUCT(BlueprintType)
struct PROJECTC_API FAccountGameServerInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Server Info")
    int32 ServerID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Server Info")
    FString ServerName;

    UPROPERTY(BlueprintReadWrite, Category = "Server Info")
    FString ServerIP;

    UPROPERTY(BlueprintReadWrite, Category = "Server Info")
    int32 ServerPort = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Server Info")
    int32 OwnerUserID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Server Info")
    FString OwnerNickname;

    UPROPERTY(BlueprintReadWrite, Category = "Server Info")
    int32 CurrentPlayers = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Server Info")
    int32 MaxPlayers = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Server Info")
    bool bHasPassword = false;
};

USTRUCT(BlueprintType)
struct PROJECTC_API FAccountPlayerData
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Player Data")
    int32 UserID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Player Data")
    FString Username;

    UPROPERTY(BlueprintReadWrite, Category = "Player Data")
    FString Nickname;

    UPROPERTY(BlueprintReadWrite, Category = "Player Data")
    int32 Level = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Player Data")
    int32 Experience = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Player Data")
    int32 Gold = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Player Data")
    int32 Health = 100;

    UPROPERTY(BlueprintReadWrite, Category = "Player Data")
    int32 Mana = 100;
};

USTRUCT(BlueprintType)
struct PROJECTC_API FAccountItemInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 ItemID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    FString ItemName;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 ItemType = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 BasePrice = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 AttackBonus = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 DefenseBonus = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 HPBonus = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 MPBonus = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    FString Description;
};

// === FlatBuffers ����ü�� ���� ���� ===
struct DatabasePacket;

/**
 * UE5�� Ŭ���̾�Ʈ ��Ŷ �Ŵ���
 * Account Server���� ��� ��Ŷ ����� ���
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTC_API UClientPacketManager : public UObject
{
    GENERATED_BODY()

public:
    UClientPacketManager();

    // === Ŭ���̾�Ʈ ��û ��Ŷ ���� (C2S) ===

    // �α��� ��û ��Ŷ ����
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Login")
    TArray<uint8> CreateLoginRequest(const FString& Username, const FString& Password);

    // �α׾ƿ� ��û ��Ŷ ����
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Login")
    TArray<uint8> CreateLogoutRequest(int32 UserID);

    // ���� ���� ��û ��Ŷ ����
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Account")
    TArray<uint8> CreateAccountRequest(const FString& Username, const FString& Password, const FString& Nickname);

    // �÷��̾� ������ ��û ��Ŷ ����
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Player Data")
    TArray<uint8> CreatePlayerDataRequest(int32 UserID);

    // ������ ������ ��û ��Ŷ ����
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Item")
    TArray<uint8> CreateItemDataRequest(int32 UserID, int32 RequestType, int32 ItemID = 0, int32 ItemCount = 0);

    // === ���� ���� ���� ��û ��Ŷ ���� (C2S) ===

    // ���� ���� ���� ��û
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    TArray<uint8> CreateGameServerRequest(int32 UserID, const FString& ServerName, const FString& ServerPassword, const FString& ServerIP, int32 ServerPort, int32 MaxPlayers);

    // ���� ���� ��� ��û
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    TArray<uint8> CreateGameServerListRequest(int32 RequestType);

    // ���� ���� ���� ��û
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    TArray<uint8> CreateJoinGameServerRequest(int32 UserID, int32 ServerID, const FString& ServerPassword);

    // ���� ���� ���� ��û
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    TArray<uint8> CreateCloseGameServerRequest(int32 UserID, int32 ServerID);

    // �÷��̾� ������ ���� ��û
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    TArray<uint8> CreateSavePlayerDataRequest(int32 UserID, int32 Level, int32 Experience, int32 Health, int32 Mana, int32 Gold, float PosX, float PosY);

    // === ���� ���� ��Ŷ �Ľ� (S2C) ===

    // �α��� ���� �Ľ�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Login")
    bool ParseLoginResponse(const TArray<uint8>& Data, FAccountLoginResponse& OutResponse);

    // �α׾ƿ� ���� �Ľ�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Login")
    bool ParseLogoutResponse(const TArray<uint8>& Data, FString& OutMessage);

    // ���� ���� ���� �Ľ�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Account")
    bool ParseCreateAccountResponse(const TArray<uint8>& Data, int32& OutUserID, FString& OutMessage, bool& bOutSuccess);

    // �÷��̾� ������ ���� �Ľ�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Player Data")
    bool ParsePlayerDataResponse(const TArray<uint8>& Data, FAccountPlayerData& OutPlayerData);

    // ������ ������ ���� �Ľ�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Item")
    bool ParseItemDataResponse(const TArray<uint8>& Data, TArray<FAccountItemInfo>& OutItems);

    // ���� ���� ���� ���� �Ľ�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    bool ParseCreateGameServerResponse(const TArray<uint8>& Data, int32& OutServerID, FString& OutMessage, bool& bOutSuccess);

    // ���� ���� ��� ���� �Ľ�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    bool ParseGameServerListResponse(const TArray<uint8>& Data, TArray<FAccountGameServerInfo>& OutServerList);

    // ���� ���� ���� ���� �Ľ�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    bool ParseJoinGameServerResponse(const TArray<uint8>& Data, FString& OutServerIP, int32& OutServerPort, FString& OutMessage, bool& bOutSuccess);

    // ���� ���� ���� ���� �Ľ�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    bool ParseCloseGameServerResponse(const TArray<uint8>& Data, FString& OutMessage, bool& bOutSuccess);

    // �÷��̾� ������ ���� ���� �Ľ�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    bool ParseSavePlayerDataResponse(const TArray<uint8>& Data, FString& OutMessage, bool& bOutSuccess);

    // === ��ƿ��Ƽ �Լ��� ===

    // ��Ŷ ��ȿ�� �˻�
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Utility")
    bool IsValidPacket(const TArray<uint8>& Data);

    // ��Ŷ Ÿ�� ��������
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Utility")
    int32 GetPacketType(const TArray<uint8>& Data);

    // ��Ŷ Ÿ�� �̸� ��������
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Utility")
    FString GetPacketTypeName(int32 PacketType);

    // Ŭ���̾�Ʈ ���� ��������
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Utility")
    int32 GetClientSocket(const TArray<uint8>& Data);

    // ������ ���� �޽��� ��������
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Utility")
    FString GetLastError() const { return LastError; }

    // === ���� ���� ���� Ȯ�� �Լ��� ===

    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Validation")
    bool IsLoginSuccess(const TArray<uint8>& Data);

    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Validation")
    bool IsCreateAccountSuccess(const TArray<uint8>& Data);

    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Validation")
    bool IsPlayerDataValid(const TArray<uint8>& Data);

    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Validation")
    bool IsItemDataValid(const TArray<uint8>& Data);

    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Validation")
    bool IsCreateGameServerSuccess(const TArray<uint8>& Data);

    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Validation")
    bool IsGameServerListValid(const TArray<uint8>& Data);

    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Validation")
    bool IsJoinGameServerSuccess(const TArray<uint8>& Data);

    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Validation")
    bool IsCloseGameServerSuccess(const TArray<uint8>& Data);

    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Validation")
    bool IsSavePlayerDataSuccess(const TArray<uint8>& Data);

private:
    // ���� ����
    UPROPERTY()
    FString LastError;

    // ���� �Լ���
    void SetError(const FString& Error);
    void ClearError();
    bool VerifyPacket(const uint8* Data, int32 Size);

    // FlatBuffers ��ƿ��Ƽ
    const DatabasePacket* GetDatabasePacket(const uint8* Data) const;

    // ���ڿ� ��ȯ ��ƿ��Ƽ
    FString ConvertToFString(const flatbuffers::String* FBString) const;
    std::string ConvertToStdString(const FString& UEString) const;
};