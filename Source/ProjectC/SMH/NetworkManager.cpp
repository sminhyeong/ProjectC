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
    // Tick ��Ȱ��ȭ - �� �̻� �ʿ� ����
    PrimaryActorTick.bCanEverTick = false;

    // �⺻�� ����
    DefaultServerIP = TEXT("127.0.0.1");
    DefaultServerPort = 8080;
    ReconnectInterval = 0.1f; // 0.1�ʸ��� �翬�� �õ�
    PacketTimeout = 5.0f; // 5�� ��Ŷ ���� ���

    // �ʱ� ����
    ClientSocket = nullptr;
    ConnectionState = ENetConnectionState::Disconnected;
    CurrentServerIP = TEXT("");
    CurrentServerPort = 0;

    // ClientPacketManager ����
    ClientPacketManager = CreateDefaultSubobject<UClientPacketManager>(TEXT("ClientPacketManager"));

    // ���� ���� �ʱ�ȭ
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

    // �̹� ����Ǿ� ������ ���� ���� ����
    if (ClientSocket)
    {
        DisconnectFromServer();
    }

    CurrentServerIP = ServerIP;
    CurrentServerPort = ServerPort;

    SetConnectionState(ENetConnectionState::Connecting);

    if (CreateSocket())
    {
        // ���� �ּ� ����
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

        // ������ ���� �õ�
        bool bConnected = ClientSocket->Connect(*ServerAddress);

        if (bConnected)
        {
            UE_LOG(LogTemp, Log, TEXT("Successfully connected to server: %s:%d"), *ServerIP, ServerPort);
            SetConnectionState(ENetConnectionState::Connected);
            StopReconnectTimer(); // ���� ���� �� �翬�� Ÿ�̸� ����
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to connect to server: %s:%d - Starting reconnect timer"), *ServerIP, ServerPort);
            SetConnectionState(ENetConnectionState::Reconnecting);
            StartReconnectTimer(); // �翬�� Ÿ�̸� ����
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
    // OutResponse �ʱ�ȭ
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

    // ClientPacketManager�� ����ؼ� �α��� ��Ŷ ����
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

    // ���� ���� ���
    TArray<uint8> ResponseData;
    if (!ReceivePacketData(ResponseData, PacketTimeout))
    {
        UE_LOG(LogTemp, Error, TEXT("Login response timeout"));
        OutResponse.bSuccess = false;
        OutResponse.Message = TEXT("Login response timeout");
        return false;
    }

    // ClientPacketManager�� ����ؼ� ���� �Ľ�
    if (!ClientPacketManager->ParseLoginResponse(ResponseData, OutResponse))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse login response: %s"), *ClientPacketManager->GetLastError());
        OutResponse.bSuccess = false;
        OutResponse.Message = FString::Printf(TEXT("Failed to parse login response: %s"), *ClientPacketManager->GetLastError());
        return false;
    }

    // �α��� ��� �α� ���
    if (OutResponse.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Login Success - UserID: %d, Username: %s, Nickname: %s, Level: %d"),
            OutResponse.UserID, *OutResponse.Username, *OutResponse.Nickname, OutResponse.Level);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Login Failed - Message: %s"), *OutResponse.Message);
    }

    // ����/���п� ������� OutResponse�� ��� ������ ��������Ƿ� true ��ȯ
    // ���� ���� ���δ� OutResponse.bSuccess�� Ȯ��
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

    // ClientPacketManager�� ����ؼ� �α׾ƿ� ��Ŷ ����
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

    // ���� ���� ���
    TArray<uint8> ResponseData;
    if (!ReceivePacketData(ResponseData, PacketTimeout))
    {
        UE_LOG(LogTemp, Error, TEXT("Logout response timeout"));
        return false;
    }

    // ClientPacketManager�� ����ؼ� ���� �Ľ�
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
            true // �ݺ�
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

    // ���� ���� ����
    CleanupSocket();

    if (CreateSocket())
    {
        // ���� �ּ� ����
        ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        TSharedRef<FInternetAddr> ServerAddress = SocketSubsystem->CreateInternetAddr();

        bool bIsValidIP = false;
        ServerAddress->SetIp(*CurrentServerIP, bIsValidIP);
        ServerAddress->SetPort(CurrentServerPort);

        if (bIsValidIP)
        {
            // ������ ���� �õ�
            bool bConnected = ClientSocket->Connect(*ServerAddress);

            if (bConnected)
            {
                UE_LOG(LogTemp, Log, TEXT("Reconnection successful!"));
                SetConnectionState(ENetConnectionState::Connected);
                StopReconnectTimer(); // ���� ���� �� Ÿ�̸� ����
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

    // TCP ���� ����
    ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("NetworkManager Socket"), false);

    if (!ClientSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create socket"));
        return false;
    }

    // ���� �ɼ� ����
    int32 SendBufferSize = 32 * 1024; // 32KB
    int32 RecvBufferSize = 32 * 1024; // 32KB

    ClientSocket->SetSendBufferSize(SendBufferSize, SendBufferSize);
    ClientSocket->SetReceiveBufferSize(RecvBufferSize, RecvBufferSize);

    // ���ŷ ��� ���� (���� ����ŷ���� ������ �� ����)
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

    // ��Ŷ ũ�� ��� �߰� (4����Ʈ) + ���� ��Ŷ ������
    uint32 PacketSize = PacketData.Num();
    uint32 NetworkSize = NETWORK_ORDER32(PacketSize); // ��Ʈ��ũ ����Ʈ ������ ��ȯ

    // ���� ������ ������ ���� (ũ�� ��� + ��Ŷ ������)
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

    // ��Ŷ ���(ũ��) ���� ����
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
                    // ��Ʈ��ũ ����Ʈ �������� ȣ��Ʈ ����Ʈ ������ ��ȯ
                    PacketSize = NETWORK_ORDER32(PacketSize);
                    bHeaderReceived = true;
                    UE_LOG(LogTemp, Log, TEXT("Received packet header, size: %d bytes"), PacketSize);
                }
            }
        }
        FPlatformProcess::Sleep(0.001f); // 1ms ���
    }

    if (!bHeaderReceived)
    {
        UE_LOG(LogTemp, Error, TEXT("Packet header receive timeout"));
        return false;
    }

    if (PacketSize == 0 || PacketSize > 65536) // �ִ� 64KB ����
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid packet size: %d"), PacketSize);
        return false;
    }

    // ���� ��Ŷ ������ ����
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
        FPlatformProcess::Sleep(0.001f); // 1ms ���
    }

    if (TotalReceived != static_cast<int32>(PacketSize))
    {
        UE_LOG(LogTemp, Error, TEXT("Incomplete packet received: %d/%d bytes"), TotalReceived, PacketSize);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Successfully received complete packet: %d bytes"), PacketSize);
    return true;
}