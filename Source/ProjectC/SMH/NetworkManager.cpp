#include "NetworkManager.h"
#include "ClientPacketManager.h"
#include "SocketSubsystem.h"
#include "Common/TcpSocketBuilder.h"
#include "IPAddress.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

ANetworkManager::ANetworkManager()
{
    // Tick 비활성화 - 더 이상 필요 없음
    PrimaryActorTick.bCanEverTick = false;

    // 기본값 설정
    DefaultServerIP = TEXT("127.0.0.1");
    DefaultServerPort = 8080;
    ReconnectInterval = 0.1f; // 0.1초마다 재연결 시도
    PacketTimeout = 5.0f; // 5초 패킷 응답 대기

    // 초기 상태
    ClientSocket = nullptr;
    ConnectionState = ENetConnectionState::Disconnected;
    CurrentServerIP = TEXT("");
    CurrentServerPort = 0;

    // ClientPacketManager 생성
    ClientPacketManager = CreateDefaultSubobject<UClientPacketManager>(TEXT("ClientPacketManager"));

    // 수신 버퍼 초기화
    ReceiveBuffer.Reserve(RECEIVE_BUFFER_SIZE);
}

void ANetworkManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("NetworkManager BeginPlay"));
}

void ANetworkManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopReconnectTimer();
    DisconnectFromServer();
    Super::EndPlay(EndPlayReason);
}

void ANetworkManager::Destroyed()
{
    StopReconnectTimer();
    CleanupSocket();
    Super::Destroyed();
}

void ANetworkManager::ConnectToServer(const FString& ServerIP, int32 ServerPort)
{
    UE_LOG(LogTemp, Log, TEXT("Attempting to connect to server: %s:%d"), *ServerIP, ServerPort);

    // 이미 연결되어 있으면 기존 연결 해제
    if (ClientSocket)
    {
        DisconnectFromServer();
    }

    CurrentServerIP = ServerIP;
    CurrentServerPort = ServerPort;

    SetConnectionState(ENetConnectionState::Connecting);

    if (CreateSocket())
    {
        // 서버 주소 생성
        ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        TSharedRef<FInternetAddr> ServerAddress = SocketSubsystem->CreateInternetAddr();

        bool bIsValidIP = false;
        ServerAddress->SetIp(*ServerIP, bIsValidIP);
        ServerAddress->SetPort(ServerPort);

        if (!bIsValidIP)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid IP address: %s"), *ServerIP);
            SetConnectionState(ENetConnectionState::Failed);
            CleanupSocket();
            return;
        }

        // 서버에 연결 시도
        bool bConnected = ClientSocket->Connect(*ServerAddress);

        if (bConnected)
        {
            UE_LOG(LogTemp, Log, TEXT("Successfully connected to server: %s:%d"), *ServerIP, ServerPort);
            SetConnectionState(ENetConnectionState::Connected);
            StopReconnectTimer(); // 연결 성공 시 재연결 타이머 중지
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to connect to server: %s:%d - Starting reconnect timer"), *ServerIP, ServerPort);
            SetConnectionState(ENetConnectionState::Reconnecting);
            StartReconnectTimer(); // 재연결 타이머 시작
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create socket"));
        SetConnectionState(ENetConnectionState::Failed);
    }
}

void ANetworkManager::DisconnectFromServer()
{
    if (ClientSocket)
    {
        UE_LOG(LogTemp, Log, TEXT("Disconnecting from server"));
        StopReconnectTimer();
        CleanupSocket();
        SetConnectionState(ENetConnectionState::Disconnected);
    }
}

bool ANetworkManager::IsConnectedToServer() const
{
    return ConnectionState == ENetConnectionState::Connected && ClientSocket != nullptr;
}

ENetConnectionState ANetworkManager::GetConnectionState() const
{
    return ConnectionState;
}

bool ANetworkManager::LoginToServer(const FString& Username, const FString& Password, FAccountLoginResponse& OutResponse)
{
    // OutResponse 초기화
    OutResponse = FAccountLoginResponse();

    if (!IsConnectedToServer())
    {
        UE_LOG(LogTemp, Error, TEXT("Not connected to server"));
        OutResponse.bSuccess = false;
        OutResponse.Message = TEXT("Not connected to server");
        return false;
    }

    if (!ClientPacketManager)
    {
        UE_LOG(LogTemp, Error, TEXT("ClientPacketManager is null"));
        OutResponse.bSuccess = false;
        OutResponse.Message = TEXT("ClientPacketManager is null");
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Attempting login: %s"), *Username);

    // ClientPacketManager를 사용해서 로그인 패킷 생성
    TArray<uint8> LoginPacket = ClientPacketManager->CreateLoginRequest(Username, Password);
    if (LoginPacket.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create login packet: %s"), *ClientPacketManager->GetLastError());
        OutResponse.bSuccess = false;
        OutResponse.Message = FString::Printf(TEXT("Failed to create login packet: %s"), *ClientPacketManager->GetLastError());
        return false;
    }

    if (!SendPacketData(LoginPacket))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to send login packet"));
        OutResponse.bSuccess = false;
        OutResponse.Message = TEXT("Failed to send login packet");
        return false;
    }

    // 서버 응답 대기
    TArray<uint8> ResponseData;
    if (!ReceivePacketData(ResponseData, PacketTimeout))
    {
        UE_LOG(LogTemp, Error, TEXT("Login response timeout"));
        OutResponse.bSuccess = false;
        OutResponse.Message = TEXT("Login response timeout");
        return false;
    }

    // ClientPacketManager를 사용해서 응답 파싱
    if (!ClientPacketManager->ParseLoginResponse(ResponseData, OutResponse))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse login response: %s"), *ClientPacketManager->GetLastError());
        OutResponse.bSuccess = false;
        OutResponse.Message = FString::Printf(TEXT("Failed to parse login response: %s"), *ClientPacketManager->GetLastError());
        return false;
    }

    // 로그인 결과 로그 출력
    if (OutResponse.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Login Success - UserID: %d, Username: %s, Nickname: %s, Level: %d"),
            OutResponse.UserID, *OutResponse.Username, *OutResponse.Nickname, OutResponse.Level);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Login Failed - Message: %s"), *OutResponse.Message);
    }

    // 성공/실패와 관계없이 OutResponse에 모든 정보가 담겨있으므로 true 반환
    // 실제 성공 여부는 OutResponse.bSuccess로 확인
    return true;
}

bool ANetworkManager::LogoutFromServer(int32 UserID, FString& OutMessage)
{
    if (!IsConnectedToServer())
    {
        UE_LOG(LogTemp, Error, TEXT("Not connected to server"));
        return false;
    }

    if (!ClientPacketManager)
    {
        UE_LOG(LogTemp, Error, TEXT("ClientPacketManager is null"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Attempting logout for UserID: %d"), UserID);

    // ClientPacketManager를 사용해서 로그아웃 패킷 생성
    TArray<uint8> LogoutPacket = ClientPacketManager->CreateLogoutRequest(UserID);
    if (LogoutPacket.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create logout packet: %s"), *ClientPacketManager->GetLastError());
        return false;
    }

    if (!SendPacketData(LogoutPacket))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to send logout packet"));
        return false;
    }

    // 서버 응답 대기
    TArray<uint8> ResponseData;
    if (!ReceivePacketData(ResponseData, PacketTimeout))
    {
        UE_LOG(LogTemp, Error, TEXT("Logout response timeout"));
        return false;
    }

    // ClientPacketManager를 사용해서 응답 파싱
    if (!ClientPacketManager->ParseLogoutResponse(ResponseData, OutMessage))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse logout response: %s"), *ClientPacketManager->GetLastError());
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Logout result: %s"), *OutMessage);
    return true;
}

void ANetworkManager::SetConnectionState(ENetConnectionState NewState)
{
    if (ConnectionState != NewState)
    {
        ConnectionState = NewState;
        OnConnectionStateChanged.Broadcast(NewState);

        UE_LOG(LogTemp, Log, TEXT("Connection state changed to: %d"), (int32)NewState);
    }
}

void ANetworkManager::StartReconnectTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ReconnectTimerHandle,
            this,
            &ANetworkManager::TryReconnect,
            ReconnectInterval,
            true // 반복
        );
        UE_LOG(LogTemp, Log, TEXT("Reconnect timer started (%.1f seconds interval)"), ReconnectInterval);
    }
}

void ANetworkManager::StopReconnectTimer()
{
    if (GetWorld() && ReconnectTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("Reconnect timer stopped"));
    }
}

void ANetworkManager::TryReconnect()
{
    if (ConnectionState != ENetConnectionState::Reconnecting)
    {
        StopReconnectTimer();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Attempting to reconnect to server: %s:%d"), *CurrentServerIP, CurrentServerPort);

    // 기존 소켓 정리
    CleanupSocket();

    if (CreateSocket())
    {
        // 서버 주소 생성
        ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        TSharedRef<FInternetAddr> ServerAddress = SocketSubsystem->CreateInternetAddr();

        bool bIsValidIP = false;
        ServerAddress->SetIp(*CurrentServerIP, bIsValidIP);
        ServerAddress->SetPort(CurrentServerPort);

        if (bIsValidIP)
        {
            // 서버에 연결 시도
            bool bConnected = ClientSocket->Connect(*ServerAddress);

            if (bConnected)
            {
                UE_LOG(LogTemp, Log, TEXT("Reconnection successful!"));
                SetConnectionState(ENetConnectionState::Connected);
                StopReconnectTimer(); // 연결 성공 시 타이머 중지
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Reconnection failed, will try again..."));
                CleanupSocket();
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid IP address during reconnection"));
            CleanupSocket();
        }
    }
}

bool ANetworkManager::CreateSocket()
{
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

    if (!SocketSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get socket subsystem"));
        return false;
    }

    // TCP 소켓 생성
    ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("NetworkManager Socket"), false);

    if (!ClientSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create socket"));
        return false;
    }

    // 소켓 옵션 설정
    int32 SendBufferSize = 32 * 1024; // 32KB
    int32 RecvBufferSize = 32 * 1024; // 32KB

    ClientSocket->SetSendBufferSize(SendBufferSize, SendBufferSize);
    ClientSocket->SetReceiveBufferSize(RecvBufferSize, RecvBufferSize);

    // 블로킹 모드 설정 (추후 논블로킹으로 변경할 수 있음)
    ClientSocket->SetNonBlocking(false);

    UE_LOG(LogTemp, Log, TEXT("Socket created successfully"));
    return true;
}

void ANetworkManager::CleanupSocket()
{
    if (ClientSocket)
    {
        ClientSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
        ClientSocket = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Socket cleaned up"));
    }
}

bool ANetworkManager::SendPacketData(const TArray<uint8>& PacketData)
{
    if (!ClientSocket || !IsConnectedToServer())
    {
        UE_LOG(LogTemp, Error, TEXT("Socket not available for sending"));
        return false;
    }

    // 패킷 크기 헤더 추가 (4바이트) + 실제 패킷 데이터
    uint32 PacketSize = PacketData.Num();
    uint32 NetworkSize = NETWORK_ORDER32(PacketSize); // 네트워크 바이트 순서로 변환

    // 최종 전송할 데이터 구성 (크기 헤더 + 패킷 데이터)
    TArray<uint8> FinalData;
    FinalData.Append(reinterpret_cast<const uint8*>(&NetworkSize), sizeof(uint32));
    FinalData.Append(PacketData);

    int32 BytesSent = 0;
    bool bSuccess = ClientSocket->Send(FinalData.GetData(), FinalData.Num(), BytesSent);

    if (bSuccess && BytesSent == FinalData.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("Sent packet: %d bytes (including %d byte header)"), BytesSent, sizeof(uint32));
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to send packet. Sent: %d/%d bytes"), BytesSent, FinalData.Num());
        return false;
    }
}

bool ANetworkManager::ReceivePacketData(TArray<uint8>& OutPacketData, float TimeoutSeconds)
{
    if (!ClientSocket || !IsConnectedToServer())
    {
        UE_LOG(LogTemp, Error, TEXT("Socket not available for receiving"));
        return false;
    }

    float StartTime = FPlatformTime::Seconds();
    uint32 PacketSize = 0;
    bool bHeaderReceived = false;

    UE_LOG(LogTemp, Log, TEXT("Waiting for packet header..."));

    // 패킷 헤더(크기) 먼저 수신
    while (!bHeaderReceived && (FPlatformTime::Seconds() - StartTime) < TimeoutSeconds)
    {
        uint32 PendingDataSize = 0;
        if (ClientSocket->HasPendingData(PendingDataSize) && PendingDataSize >= sizeof(uint32))
        {
            int32 BytesRead = 0;
            if (ClientSocket->Recv(reinterpret_cast<uint8*>(&PacketSize), sizeof(uint32), BytesRead))
            {
                if (BytesRead == sizeof(uint32))
                {
                    // 네트워크 바이트 순서에서 호스트 바이트 순서로 변환
                    PacketSize = NETWORK_ORDER32(PacketSize);
                    bHeaderReceived = true;
                    UE_LOG(LogTemp, Log, TEXT("Received packet header, size: %d bytes"), PacketSize);
                }
            }
        }
        FPlatformProcess::Sleep(0.001f); // 1ms 대기
    }

    if (!bHeaderReceived)
    {
        UE_LOG(LogTemp, Error, TEXT("Packet header receive timeout"));
        return false;
    }

    if (PacketSize == 0 || PacketSize > 65536) // 최대 64KB 제한
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid packet size: %d"), PacketSize);
        return false;
    }

    // 실제 패킷 데이터 수신
    OutPacketData.SetNum(PacketSize);
    int32 TotalReceived = 0;

    UE_LOG(LogTemp, Log, TEXT("Receiving packet data..."));

    while (TotalReceived < static_cast<int32>(PacketSize) && (FPlatformTime::Seconds() - StartTime) < TimeoutSeconds)
    {
        uint32 PendingDataSize = 0;
        if (ClientSocket->HasPendingData(PendingDataSize) && PendingDataSize > 0)
        {
            int32 BytesRead = 0;
            int32 BytesToRead = FMath::Min(static_cast<int32>(PendingDataSize), static_cast<int32>(PacketSize) - TotalReceived);

            if (ClientSocket->Recv(OutPacketData.GetData() + TotalReceived, BytesToRead, BytesRead))
            {
                TotalReceived += BytesRead;
                UE_LOG(LogTemp, VeryVerbose, TEXT("Received %d bytes, total: %d/%d"), BytesRead, TotalReceived, PacketSize);
            }
        }
        FPlatformProcess::Sleep(0.001f); // 1ms 대기
    }

    if (TotalReceived != static_cast<int32>(PacketSize))
    {
        UE_LOG(LogTemp, Error, TEXT("Incomplete packet received: %d/%d bytes"), TotalReceived, PacketSize);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Successfully received complete packet: %d bytes"), PacketSize);
    return true;
}