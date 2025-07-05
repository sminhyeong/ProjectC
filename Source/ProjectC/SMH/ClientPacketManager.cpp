// ClientPacketManager.cpp - UE5 전용 (네트워크 의존성 제거)

#include "ClientPacketManager.h"
#include "../UserEvent_generated.h"

UClientPacketManager::UClientPacketManager()
{
    ClearError();
}

void UClientPacketManager::SetError(const FString& Error)
{
    LastError = Error;
    UE_LOG(LogTemp, Error, TEXT("[ClientPacketManager] %s"), *Error);
}

void UClientPacketManager::ClearError()
{
    LastError = TEXT("");
}

bool UClientPacketManager::VerifyPacket(const uint8* Data, int32 Size)
{
    if (!Data || Size <= 0)
    {
        SetError(TEXT("Invalid packet data"));
        return false;
    }

    flatbuffers::Verifier verifier(Data, Size);
    return VerifyDatabasePacketBuffer(verifier);
}

// *** 수정: 무한재귀 버그 수정 ***
const DatabasePacket* UClientPacketManager::GetDatabasePacket(const uint8* Data) const
{
    return flatbuffers::GetRoot<DatabasePacket>(Data);
}

FString UClientPacketManager::ConvertToFString(const flatbuffers::String* FBString) const
{
    if (!FBString)
    {
        return FString();
    }
    return FString(UTF8_TO_TCHAR(FBString->c_str()));
}

std::string UClientPacketManager::ConvertToStdString(const FString& UEString) const
{
    return std::string(TCHAR_TO_UTF8(*UEString));
}

// === 클라이언트 요청 패킷 생성 (C2S) ===

TArray<uint8> UClientPacketManager::CreateLoginRequest(const FString& Username, const FString& Password)
{
    ClearError();

    try
    {
        flatbuffers::FlatBufferBuilder builder;

        auto usernameOffset = builder.CreateString(ConvertToStdString(Username));
        auto passwordOffset = builder.CreateString(ConvertToStdString(Password));

        auto loginRequest = CreateC2S_Login(builder, usernameOffset, passwordOffset);
        auto packet = CreateDatabasePacket(builder, EventType_C2S_Login, loginRequest.Union());

        builder.Finish(packet);

        TArray<uint8> PacketData;
        uint8* bufferPointer = builder.GetBufferPointer();
        int32 bufferSize = static_cast<int32>(builder.GetSize());

        PacketData.Append(bufferPointer, bufferSize);

        UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] CreateLoginRequest: %s, Size: %d"), *Username, PacketData.Num());
        return PacketData;
    }
    catch (const std::exception& e)
    {
        SetError(FString::Printf(TEXT("CreateLoginRequest failed: %s"), UTF8_TO_TCHAR(e.what())));
        return TArray<uint8>();
    }
}

TArray<uint8> UClientPacketManager::CreateLogoutRequest(int32 UserID)
{
    ClearError();

    try
    {
        flatbuffers::FlatBufferBuilder builder;

        auto logoutRequest = CreateC2S_Logout(builder, static_cast<uint32_t>(UserID));
        auto packet = CreateDatabasePacket(builder, EventType_C2S_Logout, logoutRequest.Union());

        builder.Finish(packet);

        TArray<uint8> PacketData;
        uint8* bufferPointer = builder.GetBufferPointer();
        int32 bufferSize = static_cast<int32>(builder.GetSize());

        PacketData.Append(bufferPointer, bufferSize);

        UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] CreateLogoutRequest: UserID %d, Size: %d"), UserID, PacketData.Num());
        return PacketData;
    }
    catch (const std::exception& e)
    {
        SetError(FString::Printf(TEXT("CreateLogoutRequest failed: %s"), UTF8_TO_TCHAR(e.what())));
        return TArray<uint8>();
    }
}

TArray<uint8> UClientPacketManager::CreateAccountRequest(const FString& Username, const FString& Password, const FString& Nickname)
{
    ClearError();

    try
    {
        flatbuffers::FlatBufferBuilder builder;

        auto usernameOffset = builder.CreateString(ConvertToStdString(Username));
        auto passwordOffset = builder.CreateString(ConvertToStdString(Password));
        auto nicknameOffset = builder.CreateString(ConvertToStdString(Nickname));

        auto createAccountRequest = CreateC2S_CreateAccount(builder, usernameOffset, passwordOffset, nicknameOffset);
        auto packet = CreateDatabasePacket(builder, EventType_C2S_CreateAccount, createAccountRequest.Union());

        builder.Finish(packet);

        TArray<uint8> PacketData;
        uint8* bufferPointer = builder.GetBufferPointer();
        int32 bufferSize = static_cast<int32>(builder.GetSize());

        PacketData.Append(bufferPointer, bufferSize);

        UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] CreateAccountRequest: %s, Size: %d"), *Username, PacketData.Num());
        return PacketData;
    }
    catch (const std::exception& e)
    {
        SetError(FString::Printf(TEXT("CreateAccountRequest failed: %s"), UTF8_TO_TCHAR(e.what())));
        return TArray<uint8>();
    }
}

TArray<uint8> UClientPacketManager::CreatePlayerDataRequest(int32 UserID)
{
    ClearError();

    try
    {
        flatbuffers::FlatBufferBuilder builder;

        auto playerDataRequest = CreateC2S_PlayerData(builder, static_cast<uint32_t>(UserID), 0); // 0 = 조회 요청
        auto packet = CreateDatabasePacket(builder, EventType_C2S_PlayerData, playerDataRequest.Union());

        builder.Finish(packet);

        TArray<uint8> PacketData;
        uint8* bufferPointer = builder.GetBufferPointer();
        int32 bufferSize = static_cast<int32>(builder.GetSize());

        PacketData.Append(bufferPointer, bufferSize);

        UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] CreatePlayerDataRequest: UserID %d, Size: %d"), UserID, PacketData.Num());
        return PacketData;
    }
    catch (const std::exception& e)
    {
        SetError(FString::Printf(TEXT("CreatePlayerDataRequest failed: %s"), UTF8_TO_TCHAR(e.what())));
        return TArray<uint8>();
    }
}

TArray<uint8> UClientPacketManager::CreateItemDataRequest(int32 UserID, int32 RequestType, int32 ItemID, int32 ItemCount)
{
    ClearError();

    try
    {
        flatbuffers::FlatBufferBuilder builder;

        auto itemDataRequest = CreateC2S_ItemData(builder,
            static_cast<uint32_t>(UserID),
            static_cast<uint32_t>(RequestType),
            static_cast<uint32_t>(ItemID),
            static_cast<uint32_t>(ItemCount));
        auto packet = CreateDatabasePacket(builder, EventType_C2S_ItemData, itemDataRequest.Union());

        builder.Finish(packet);

        TArray<uint8> PacketData;
        uint8* bufferPointer = builder.GetBufferPointer();
        int32 bufferSize = static_cast<int32>(builder.GetSize());

        PacketData.Append(bufferPointer, bufferSize);

        UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] CreateItemDataRequest: UserID %d, Type %d, ItemID %d, Count %d, Size: %d"),
            UserID, RequestType, ItemID, ItemCount, PacketData.Num());
        return PacketData;
    }
    catch (const std::exception& e)
    {
        SetError(FString::Printf(TEXT("CreateItemDataRequest failed: %s"), UTF8_TO_TCHAR(e.what())));
        return TArray<uint8>();
    }
}

// === 게임 서버 관련 패킷 생성 함수들 ===

TArray<uint8> UClientPacketManager::CreateGameServerRequest(int32 UserID, const FString& ServerName, const FString& ServerPassword, const FString& ServerIP, int32 ServerPort, int32 MaxPlayers)
{
    ClearError();

    try
    {
        flatbuffers::FlatBufferBuilder builder;

        auto serverNameOffset = builder.CreateString(ConvertToStdString(ServerName));
        auto serverPasswordOffset = builder.CreateString(ConvertToStdString(ServerPassword));
        auto serverIpOffset = builder.CreateString(ConvertToStdString(ServerIP));

        auto gameServerRequest = CreateC2S_CreateGameServer(builder,
            static_cast<uint32_t>(UserID),
            serverNameOffset,
            serverPasswordOffset,
            serverIpOffset,
            static_cast<uint32_t>(ServerPort),
            static_cast<uint32_t>(MaxPlayers));
        auto packet = CreateDatabasePacket(builder, EventType_C2S_CreateGameServer, gameServerRequest.Union());

        builder.Finish(packet);

        TArray<uint8> PacketData;
        uint8* bufferPointer = builder.GetBufferPointer();
        int32 bufferSize = static_cast<int32>(builder.GetSize());

        PacketData.Append(bufferPointer, bufferSize);

        UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] CreateGameServerRequest: %s, Size: %d"), *ServerName, PacketData.Num());
        return PacketData;
    }
    catch (const std::exception& e)
    {
        SetError(FString::Printf(TEXT("CreateGameServerRequest failed: %s"), UTF8_TO_TCHAR(e.what())));
        return TArray<uint8>();
    }
}

TArray<uint8> UClientPacketManager::CreateGameServerListRequest(int32 RequestType)
{
    ClearError();

    try
    {
        flatbuffers::FlatBufferBuilder builder;

        auto gameServerListRequest = CreateC2S_GameServerList(builder, static_cast<uint32_t>(RequestType));
        auto packet = CreateDatabasePacket(builder, EventType_C2S_GameServerList, gameServerListRequest.Union());

        builder.Finish(packet);

        TArray<uint8> PacketData;
        uint8* bufferPointer = builder.GetBufferPointer();
        int32 bufferSize = static_cast<int32>(builder.GetSize());

        PacketData.Append(bufferPointer, bufferSize);

        UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] CreateGameServerListRequest: Type %d, Size: %d"), RequestType, PacketData.Num());
        return PacketData;
    }
    catch (const std::exception& e)
    {
        SetError(FString::Printf(TEXT("CreateGameServerListRequest failed: %s"), UTF8_TO_TCHAR(e.what())));
        return TArray<uint8>();
    }
}

TArray<uint8> UClientPacketManager::CreateJoinGameServerRequest(int32 UserID, int32 ServerID, const FString& ServerPassword)
{
    ClearError();

    try
    {
        flatbuffers::FlatBufferBuilder builder;

        auto serverPasswordOffset = builder.CreateString(ConvertToStdString(ServerPassword));

        auto joinGameServerRequest = CreateC2S_JoinGameServer(builder,
            static_cast<uint32_t>(UserID),
            static_cast<uint32_t>(ServerID),
            serverPasswordOffset);
        auto packet = CreateDatabasePacket(builder, EventType_C2S_JoinGameServer, joinGameServerRequest.Union());

        builder.Finish(packet);

        TArray<uint8> PacketData;
        uint8* bufferPointer = builder.GetBufferPointer();
        int32 bufferSize = static_cast<int32>(builder.GetSize());

        PacketData.Append(bufferPointer, bufferSize);

        UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] CreateJoinGameServerRequest: UserID %d, ServerID %d, Size: %d"),
            UserID, ServerID, PacketData.Num());
        return PacketData;
    }
    catch (const std::exception& e)
    {
        SetError(FString::Printf(TEXT("CreateJoinGameServerRequest failed: %s"), UTF8_TO_TCHAR(e.what())));
        return TArray<uint8>();
    }
}

TArray<uint8> UClientPacketManager::CreateCloseGameServerRequest(int32 UserID, int32 ServerID)
{
    ClearError();

    try
    {
        flatbuffers::FlatBufferBuilder builder;

        auto closeGameServerRequest = CreateC2S_CloseGameServer(builder,
            static_cast<uint32_t>(UserID),
            static_cast<uint32_t>(ServerID));
        auto packet = CreateDatabasePacket(builder, EventType_C2S_CloseGameServer, closeGameServerRequest.Union());

        builder.Finish(packet);

        TArray<uint8> PacketData;
        uint8* bufferPointer = builder.GetBufferPointer();
        int32 bufferSize = static_cast<int32>(builder.GetSize());

        PacketData.Append(bufferPointer, bufferSize);

        UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] CreateCloseGameServerRequest: UserID %d, ServerID %d, Size: %d"),
            UserID, ServerID, PacketData.Num());
        return PacketData;
    }
    catch (const std::exception& e)
    {
        SetError(FString::Printf(TEXT("CreateCloseGameServerRequest failed: %s"), UTF8_TO_TCHAR(e.what())));
        return TArray<uint8>();
    }
}

TArray<uint8> UClientPacketManager::CreateSavePlayerDataRequest(int32 UserID, int32 Level, int32 Experience, int32 Health, int32 Mana, int32 Gold, float PosX, float PosY)
{
    ClearError();

    try
    {
        flatbuffers::FlatBufferBuilder builder;

        auto savePlayerDataRequest = CreateC2S_SavePlayerData(builder,
            static_cast<uint32_t>(UserID),
            static_cast<uint32_t>(Level),
            static_cast<uint32_t>(Experience),
            static_cast<uint32_t>(Health),
            static_cast<uint32_t>(Mana),
            static_cast<uint32_t>(Gold),
            PosX,
            PosY);
        auto packet = CreateDatabasePacket(builder, EventType_C2S_SavePlayerData, savePlayerDataRequest.Union());

        builder.Finish(packet);

        TArray<uint8> PacketData;
        uint8* bufferPointer = builder.GetBufferPointer();
        int32 bufferSize = static_cast<int32>(builder.GetSize());

        PacketData.Append(bufferPointer, bufferSize);

        UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] CreateSavePlayerDataRequest: UserID %d, Level %d, Size: %d"),
            UserID, Level, PacketData.Num());
        return PacketData;
    }
    catch (const std::exception& e)
    {
        SetError(FString::Printf(TEXT("CreateSavePlayerDataRequest failed: %s"), UTF8_TO_TCHAR(e.what())));
        return TArray<uint8>();
    }
}

// === 서버 응답 패킷 파싱 (S2C) ===

bool UClientPacketManager::ParseLoginResponse(const TArray<uint8>& Data, FAccountLoginResponse& OutResponse)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_Login)
    {
        SetError(TEXT("Invalid login response packet"));
        return false;
    }

    const S2C_Login* loginResponse = packet->packet_event_as_S2C_Login();
    if (!loginResponse)
    {
        SetError(TEXT("Failed to cast login response"));
        return false;
    }

    // 데이터 파싱
    OutResponse.bSuccess = (loginResponse->result() == ResultCode_SUCCESS);
    OutResponse.UserID = static_cast<int32>(loginResponse->user_id());
    OutResponse.Username = ConvertToFString(loginResponse->username());
    OutResponse.Nickname = ConvertToFString(loginResponse->nickname());
    OutResponse.Level = static_cast<int32>(loginResponse->level());
    OutResponse.Message = OutResponse.bSuccess ? TEXT("한글 Success") : TEXT("Login Failed");

    // FString::Printf로 로그 메시지 생성
    FString LogMsg = FString::Printf(TEXT("[ClientPacketManager] ParseLoginResponse: Success=%s, UserID=%d, Username=%s"), OutResponse.bSuccess ? TEXT("true") : TEXT("false"), OutResponse.UserID, *OutResponse.Username);
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMsg);

    return true;
}

bool UClientPacketManager::ParseLogoutResponse(const TArray<uint8>& Data, FString& OutMessage)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_Logout)
    {
        SetError(TEXT("Invalid logout response packet"));
        return false;
    }

    const S2C_Logout* logoutResponse = packet->packet_event_as_S2C_Logout();
    if (!logoutResponse)
    {
        SetError(TEXT("Failed to cast logout response"));
        return false;
    }

    // S2C_Logout에는 message() 있음 - 유지
    OutMessage = ConvertToFString(logoutResponse->message());

    UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] ParseLogoutResponse: %s"), *OutMessage);
    return true;
}

bool UClientPacketManager::ParseCreateAccountResponse(const TArray<uint8>& Data, int32& OutUserID, FString& OutMessage, bool& bOutSuccess)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_CreateAccount)
    {
        SetError(TEXT("Invalid create account response packet"));
        return false;
    }

    const S2C_CreateAccount* createAccountResponse = packet->packet_event_as_S2C_CreateAccount();
    if (!createAccountResponse)
    {
        SetError(TEXT("Failed to cast create account response"));
        return false;
    }

    // S2C_CreateAccount에는 message() 있음 - 유지
    bOutSuccess = (createAccountResponse->result() == ResultCode_SUCCESS);
    OutUserID = static_cast<int32>(createAccountResponse->user_id());
    OutMessage = ConvertToFString(createAccountResponse->message());

    UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] ParseCreateAccountResponse: Success=%s, UserID=%d, Message=%s"),
        bOutSuccess ? TEXT("true") : TEXT("false"), OutUserID, *OutMessage);

    return true;
}

bool UClientPacketManager::ParsePlayerDataResponse(const TArray<uint8>& Data, FAccountPlayerData& OutPlayerData)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_PlayerData)
    {
        SetError(TEXT("Invalid player data response packet"));
        return false;
    }

    const S2C_PlayerData* playerDataResponse = packet->packet_event_as_S2C_PlayerData();
    if (!playerDataResponse)
    {
        SetError(TEXT("Failed to cast player data response"));
        return false;
    }

    // 성공 여부 확인 (S2C_PlayerData에는 message() 없음)
    if (playerDataResponse->result() != ResultCode_SUCCESS)
    {
        SetError(TEXT("Player data request failed"));
        return false;
    }

    // FAccountPlayerData 구조체에 데이터 채우기
    OutPlayerData.UserID = static_cast<int32>(playerDataResponse->user_id());
    OutPlayerData.Username = ConvertToFString(playerDataResponse->username());
    OutPlayerData.Nickname = ConvertToFString(playerDataResponse->nickname());
    OutPlayerData.Level = static_cast<int32>(playerDataResponse->level());
    OutPlayerData.Experience = static_cast<int32>(playerDataResponse->exp());
    OutPlayerData.Gold = static_cast<int32>(playerDataResponse->gold());
    OutPlayerData.Health = static_cast<int32>(playerDataResponse->hp());
    OutPlayerData.Mana = static_cast<int32>(playerDataResponse->mp());

    UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] ParsePlayerDataResponse: UserID=%d, Level=%d, Gold=%d"),
        OutPlayerData.UserID, OutPlayerData.Level, OutPlayerData.Gold);

    return true;
}

bool UClientPacketManager::ParseItemDataResponse(const TArray<uint8>& Data, TArray<FAccountItemInfo>& OutItems)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_ItemData)
    {
        SetError(TEXT("Invalid item data response packet"));
        return false;
    }

    const S2C_ItemData* itemDataResponse = packet->packet_event_as_S2C_ItemData();
    if (!itemDataResponse)
    {
        SetError(TEXT("Failed to cast item data response"));
        return false;
    }

    // 성공 여부 확인 (S2C_ItemData에는 message() 없음)
    if (itemDataResponse->result() != ResultCode_SUCCESS)
    {
        SetError(TEXT("Item data request failed"));
        return false;
    }

    // 아이템 배열 파싱
    OutItems.Empty();
    if (itemDataResponse->items())
    {
        for (uint32 i = 0; i < itemDataResponse->items()->size(); ++i)
        {
            const ItemData* itemData = itemDataResponse->items()->Get(i);
            if (itemData)
            {
                FAccountItemInfo itemInfo;
                itemInfo.ItemID = static_cast<int32>(itemData->item_id());
                itemInfo.ItemName = ConvertToFString(itemData->item_name());
                itemInfo.ItemType = static_cast<int32>(itemData->item_type());
                itemInfo.Quantity = static_cast<int32>(itemData->item_count());
                itemInfo.BasePrice = static_cast<int32>(itemData->base_price());
                itemInfo.AttackBonus = static_cast<int32>(itemData->attack_bonus());
                itemInfo.DefenseBonus = static_cast<int32>(itemData->defense_bonus());
                itemInfo.HPBonus = static_cast<int32>(itemData->hp_bonus());
                itemInfo.MPBonus = static_cast<int32>(itemData->mp_bonus());
                itemInfo.Description = ConvertToFString(itemData->description());

                OutItems.Add(itemInfo);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] ParseItemDataResponse: %d items parsed"), OutItems.Num());
    return true;
}

// === 게임 서버 관련 파싱 함수들 ===

bool UClientPacketManager::ParseCreateGameServerResponse(const TArray<uint8>& Data, int32& OutServerID, FString& OutMessage, bool& bOutSuccess)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_CreateGameServer)
    {
        SetError(TEXT("Invalid create game server response packet"));
        return false;
    }

    const S2C_CreateGameServer* createResponse = packet->packet_event_as_S2C_CreateGameServer();
    if (!createResponse)
    {
        SetError(TEXT("Failed to cast create game server response"));
        return false;
    }

    // S2C_CreateGameServer에는 message() 있음 - 유지
    bOutSuccess = (createResponse->result() == ResultCode_SUCCESS);
    OutServerID = static_cast<int32>(createResponse->server_id());
    OutMessage = ConvertToFString(createResponse->message());

    UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] ParseCreateGameServerResponse: Success=%s, ServerID=%d"),
        bOutSuccess ? TEXT("true") : TEXT("false"), OutServerID);

    return true;
}

bool UClientPacketManager::ParseGameServerListResponse(const TArray<uint8>& Data, TArray<FAccountGameServerInfo>& OutServerList)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_GameServerList)
    {
        SetError(TEXT("Invalid game server list response packet"));
        return false;
    }

    const S2C_GameServerList* serverListResponse = packet->packet_event_as_S2C_GameServerList();
    if (!serverListResponse)
    {
        SetError(TEXT("Failed to cast game server list response"));
        return false;
    }

    // 성공 여부 확인 (S2C_GameServerList에는 message() 없음)
    if (serverListResponse->result() != ResultCode_SUCCESS)
    {
        SetError(TEXT("Game server list request failed"));
        return false;
    }

    // 서버 리스트 파싱
    OutServerList.Empty();
    if (serverListResponse->servers())
    {
        for (uint32 i = 0; i < serverListResponse->servers()->size(); ++i)
        {
            const GameServerData* serverData = serverListResponse->servers()->Get(i);
            if (serverData)
            {
                FAccountGameServerInfo serverInfo;
                serverInfo.ServerID = static_cast<int32>(serverData->server_id());
                serverInfo.ServerName = ConvertToFString(serverData->server_name());
                serverInfo.ServerIP = ConvertToFString(serverData->server_ip());
                serverInfo.ServerPort = static_cast<int32>(serverData->server_port());
                serverInfo.OwnerUserID = static_cast<int32>(serverData->owner_user_id());
                serverInfo.OwnerNickname = ConvertToFString(serverData->owner_nickname());
                serverInfo.CurrentPlayers = static_cast<int32>(serverData->current_players());
                serverInfo.MaxPlayers = static_cast<int32>(serverData->max_players());
                serverInfo.bHasPassword = serverData->has_password();

                OutServerList.Add(serverInfo);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] ParseGameServerListResponse: %d servers parsed"), OutServerList.Num());
    return true;
}

bool UClientPacketManager::ParseJoinGameServerResponse(const TArray<uint8>& Data, FString& OutServerIP, int32& OutServerPort, FString& OutMessage, bool& bOutSuccess)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_JoinGameServer)
    {
        SetError(TEXT("Invalid join game server response packet"));
        return false;
    }

    const S2C_JoinGameServer* joinResponse = packet->packet_event_as_S2C_JoinGameServer();
    if (!joinResponse)
    {
        SetError(TEXT("Failed to cast join game server response"));
        return false;
    }

    // S2C_JoinGameServer에는 message() 있음 - 유지
    bOutSuccess = (joinResponse->result() == ResultCode_SUCCESS);
    OutServerIP = ConvertToFString(joinResponse->server_ip());
    OutServerPort = static_cast<int32>(joinResponse->server_port());
    OutMessage = ConvertToFString(joinResponse->message());

    UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] ParseJoinGameServerResponse: Success=%s, IP=%s, Port=%d"),
        bOutSuccess ? TEXT("true") : TEXT("false"), *OutServerIP, OutServerPort);

    return true;
}

bool UClientPacketManager::ParseCloseGameServerResponse(const TArray<uint8>& Data, FString& OutMessage, bool& bOutSuccess)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_CloseGameServer)
    {
        SetError(TEXT("Invalid close game server response packet"));
        return false;
    }

    const S2C_CloseGameServer* closeResponse = packet->packet_event_as_S2C_CloseGameServer();
    if (!closeResponse)
    {
        SetError(TEXT("Failed to cast close game server response"));
        return false;
    }

    // S2C_CloseGameServer에는 message() 있음 - 유지
    bOutSuccess = (closeResponse->result() == ResultCode_SUCCESS);
    OutMessage = ConvertToFString(closeResponse->message());

    UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] ParseCloseGameServerResponse: Success=%s, Message=%s"),
        bOutSuccess ? TEXT("true") : TEXT("false"), *OutMessage);

    return true;
}

bool UClientPacketManager::ParseSavePlayerDataResponse(const TArray<uint8>& Data, FString& OutMessage, bool& bOutSuccess)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_SavePlayerData)
    {
        SetError(TEXT("Invalid save player data response packet"));
        return false;
    }

    const S2C_SavePlayerData* saveResponse = packet->packet_event_as_S2C_SavePlayerData();
    if (!saveResponse)
    {
        SetError(TEXT("Failed to cast save player data response"));
        return false;
    }

    // S2C_SavePlayerData에는 message() 있음 - 유지
    bOutSuccess = (saveResponse->result() == ResultCode_SUCCESS);
    OutMessage = ConvertToFString(saveResponse->message());

    UE_LOG(LogTemp, Log, TEXT("[ClientPacketManager] ParseSavePlayerDataResponse: Success=%s, Message=%s"),
        bOutSuccess ? TEXT("true") : TEXT("false"), *OutMessage);

    return true;
}

// === 유틸리티 함수들 ===

bool UClientPacketManager::IsValidPacket(const TArray<uint8>& Data)
{
    return VerifyPacket(Data.GetData(), Data.Num());
}

int32 UClientPacketManager::GetPacketType(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return static_cast<int32>(EventType_NONE);
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    return packet ? static_cast<int32>(packet->packet_event_type()) : static_cast<int32>(EventType_NONE);
}

FString UClientPacketManager::GetPacketTypeName(int32 PacketType)
{
    // EventType enum을 FString으로 변환
    switch (static_cast<EventType>(PacketType))
    {
    case EventType_C2S_Login: return TEXT("C2S_Login");
    case EventType_S2C_Login: return TEXT("S2C_Login");
    case EventType_C2S_Logout: return TEXT("C2S_Logout");
    case EventType_S2C_Logout: return TEXT("S2C_Logout");
    case EventType_C2S_CreateAccount: return TEXT("C2S_CreateAccount");
    case EventType_S2C_CreateAccount: return TEXT("S2C_CreateAccount");
    case EventType_C2S_PlayerData: return TEXT("C2S_PlayerData");
    case EventType_S2C_PlayerData: return TEXT("S2C_PlayerData");
    case EventType_C2S_ItemData: return TEXT("C2S_ItemData");
    case EventType_S2C_ItemData: return TEXT("S2C_ItemData");
    case EventType_C2S_CreateGameServer: return TEXT("C2S_CreateGameServer");
    case EventType_S2C_CreateGameServer: return TEXT("S2C_CreateGameServer");
    case EventType_C2S_GameServerList: return TEXT("C2S_GameServerList");
    case EventType_S2C_GameServerList: return TEXT("S2C_GameServerList");
    case EventType_C2S_JoinGameServer: return TEXT("C2S_JoinGameServer");
    case EventType_S2C_JoinGameServer: return TEXT("S2C_JoinGameServer");
    case EventType_C2S_CloseGameServer: return TEXT("C2S_CloseGameServer");
    case EventType_S2C_CloseGameServer: return TEXT("S2C_CloseGameServer");
    case EventType_C2S_SavePlayerData: return TEXT("C2S_SavePlayerData");
    case EventType_S2C_SavePlayerData: return TEXT("S2C_SavePlayerData");
    default: return TEXT("Unknown");
    }
}

int32 UClientPacketManager::GetClientSocket(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return 0;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    return packet ? static_cast<int32>(packet->client_socket()) : 0;
}

// === 응답 성공 여부 확인 함수들 ===

bool UClientPacketManager::IsLoginSuccess(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_Login)
    {
        return false;
    }

    const S2C_Login* loginResponse = packet->packet_event_as_S2C_Login();
    return loginResponse && (loginResponse->result() == ResultCode_SUCCESS);
}

bool UClientPacketManager::IsCreateAccountSuccess(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_CreateAccount)
    {
        return false;
    }

    const S2C_CreateAccount* createAccountResponse = packet->packet_event_as_S2C_CreateAccount();
    return createAccountResponse && (createAccountResponse->result() == ResultCode_SUCCESS);
}

bool UClientPacketManager::IsPlayerDataValid(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_PlayerData)
    {
        return false;
    }

    const S2C_PlayerData* playerDataResponse = packet->packet_event_as_S2C_PlayerData();
    return playerDataResponse && (playerDataResponse->result() == ResultCode_SUCCESS);
}

bool UClientPacketManager::IsItemDataValid(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_ItemData)
    {
        return false;
    }

    const S2C_ItemData* itemDataResponse = packet->packet_event_as_S2C_ItemData();
    return itemDataResponse && (itemDataResponse->result() == ResultCode_SUCCESS);
}

bool UClientPacketManager::IsCreateGameServerSuccess(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_CreateGameServer)
    {
        return false;
    }

    const S2C_CreateGameServer* createResponse = packet->packet_event_as_S2C_CreateGameServer();
    return createResponse && (createResponse->result() == ResultCode_SUCCESS);
}

bool UClientPacketManager::IsGameServerListValid(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_GameServerList)
    {
        return false;
    }

    const S2C_GameServerList* serverListResponse = packet->packet_event_as_S2C_GameServerList();
    return serverListResponse && (serverListResponse->result() == ResultCode_SUCCESS);
}

bool UClientPacketManager::IsJoinGameServerSuccess(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_JoinGameServer)
    {
        return false;
    }

    const S2C_JoinGameServer* joinResponse = packet->packet_event_as_S2C_JoinGameServer();
    return joinResponse && (joinResponse->result() == ResultCode_SUCCESS);
}

bool UClientPacketManager::IsCloseGameServerSuccess(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_CloseGameServer)
    {
        return false;
    }

    const S2C_CloseGameServer* closeResponse = packet->packet_event_as_S2C_CloseGameServer();
    return closeResponse && (closeResponse->result() == ResultCode_SUCCESS);
}

bool UClientPacketManager::IsSavePlayerDataSuccess(const TArray<uint8>& Data)
{
    if (!VerifyPacket(Data.GetData(), Data.Num()))
    {
        return false;
    }

    const DatabasePacket* packet = GetDatabasePacket(Data.GetData());
    if (!packet || packet->packet_event_type() != EventType_S2C_SavePlayerData)
    {
        return false;
    }

    const S2C_SavePlayerData* saveResponse = packet->packet_event_as_S2C_SavePlayerData();
    return saveResponse && (saveResponse->result() == ResultCode_SUCCESS);
}