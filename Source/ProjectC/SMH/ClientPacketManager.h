// ClientPacketManager.h - 수정된 헤더 파일
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "flatbuffers/flatbuffers.h"
#include "ClientPacketManager.generated.h"

// === UE5용 구조체 정의들 ===

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

// === FlatBuffers 구조체들 전방 선언 ===
struct DatabasePacket;

/**
 * UE5용 클라이언트 패킷 매니저
 * Account Server와의 모든 패킷 통신을 담당
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTC_API UClientPacketManager : public UObject
{
    GENERATED_BODY()

public:
    UClientPacketManager();

    // === 클라이언트 요청 패킷 생성 (C2S) ===

    // 로그인 요청 패킷 생성
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Login")
    TArray<uint8> CreateLoginRequest(const FString& Username, const FString& Password);

    // 로그아웃 요청 패킷 생성
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Login")
    TArray<uint8> CreateLogoutRequest(int32 UserID);

    // 계정 생성 요청 패킷 생성
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Account")
    TArray<uint8> CreateAccountRequest(const FString& Username, const FString& Password, const FString& Nickname);

    // 플레이어 데이터 요청 패킷 생성
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Player Data")
    TArray<uint8> CreatePlayerDataRequest(int32 UserID);

    // 아이템 데이터 요청 패킷 생성
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Item")
    TArray<uint8> CreateItemDataRequest(int32 UserID, int32 RequestType, int32 ItemID = 0, int32 ItemCount = 0);

    // === 게임 서버 관련 요청 패킷 생성 (C2S) ===

    // 게임 서버 생성 요청
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    TArray<uint8> CreateGameServerRequest(int32 UserID, const FString& ServerName, const FString& ServerPassword, const FString& ServerIP, int32 ServerPort, int32 MaxPlayers);

    // 게임 서버 목록 요청
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    TArray<uint8> CreateGameServerListRequest(int32 RequestType);

    // 게임 서버 접속 요청
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    TArray<uint8> CreateJoinGameServerRequest(int32 UserID, int32 ServerID, const FString& ServerPassword);

    // 게임 서버 종료 요청
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    TArray<uint8> CreateCloseGameServerRequest(int32 UserID, int32 ServerID);

    // 플레이어 데이터 저장 요청
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    TArray<uint8> CreateSavePlayerDataRequest(int32 UserID, int32 Level, int32 Experience, int32 Health, int32 Mana, int32 Gold, float PosX, float PosY);

    // === 서버 응답 패킷 파싱 (S2C) ===

    // 로그인 응답 파싱
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Login")
    bool ParseLoginResponse(const TArray<uint8>& Data, FAccountLoginResponse& OutResponse);

    // 로그아웃 응답 파싱
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Login")
    bool ParseLogoutResponse(const TArray<uint8>& Data, FString& OutMessage);

    // 계정 생성 응답 파싱
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Account")
    bool ParseCreateAccountResponse(const TArray<uint8>& Data, int32& OutUserID, FString& OutMessage, bool& bOutSuccess);

    // 플레이어 데이터 응답 파싱
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Player Data")
    bool ParsePlayerDataResponse(const TArray<uint8>& Data, FAccountPlayerData& OutPlayerData);

    // 아이템 데이터 응답 파싱
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Item")
    bool ParseItemDataResponse(const TArray<uint8>& Data, TArray<FAccountItemInfo>& OutItems);

    // 게임 서버 생성 응답 파싱
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    bool ParseCreateGameServerResponse(const TArray<uint8>& Data, int32& OutServerID, FString& OutMessage, bool& bOutSuccess);

    // 게임 서버 목록 응답 파싱
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    bool ParseGameServerListResponse(const TArray<uint8>& Data, TArray<FAccountGameServerInfo>& OutServerList);

    // 게임 서버 접속 응답 파싱
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    bool ParseJoinGameServerResponse(const TArray<uint8>& Data, FString& OutServerIP, int32& OutServerPort, FString& OutMessage, bool& bOutSuccess);

    // 게임 서버 종료 응답 파싱
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    bool ParseCloseGameServerResponse(const TArray<uint8>& Data, FString& OutMessage, bool& bOutSuccess);

    // 플레이어 데이터 저장 응답 파싱
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Game Server")
    bool ParseSavePlayerDataResponse(const TArray<uint8>& Data, FString& OutMessage, bool& bOutSuccess);

    // === 유틸리티 함수들 ===

    // 패킷 유효성 검사
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Utility")
    bool IsValidPacket(const TArray<uint8>& Data);

    // 패킷 타입 가져오기
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Utility")
    int32 GetPacketType(const TArray<uint8>& Data);

    // 패킷 타입 이름 가져오기
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Utility")
    FString GetPacketTypeName(int32 PacketType);

    // 클라이언트 소켓 가져오기
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Utility")
    int32 GetClientSocket(const TArray<uint8>& Data);

    // 마지막 오류 메시지 가져오기
    UFUNCTION(BlueprintCallable, Category = "Packet Manager|Utility")
    FString GetLastError() const { return LastError; }

    // === 응답 성공 여부 확인 함수들 ===

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
    // 오류 관리
    UPROPERTY()
    FString LastError;

    // 헬퍼 함수들
    void SetError(const FString& Error);
    void ClearError();
    bool VerifyPacket(const uint8* Data, int32 Size);

    // FlatBuffers 유틸리티
    const DatabasePacket* GetDatabasePacket(const uint8* Data) const;

    // 문자열 변환 유틸리티
    FString ConvertToFString(const flatbuffers::String* FBString) const;
    std::string ConvertToStdString(const FString& UEString) const;
};