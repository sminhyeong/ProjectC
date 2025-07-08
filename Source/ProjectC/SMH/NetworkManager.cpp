#ifdef _WIN32
#ifdef SetPort
#undef SetPort
#endif
#endif

#include "NetworkManager.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Engine/Engine.h"
#include "Common/TcpSocketBuilder.h"



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
		TSharedRef<FInternetAddr> ServerAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

		if (CreateServerAddress(ServerIP, ServerPort, ServerAddress))
		{
			// 논블로킹 모드로 연결 시도
			ClientSocket->SetNonBlocking(true);

			bool bConnected = ClientSocket->Connect(*ServerAddress);

			UE_LOG(LogTemp, Warning, TEXT("Connected %d"), (int)bConnected);

			if (bConnected)
			{
				// 즉시 연결 성공 (로컬 등) - 이 주석도 틀림
				UE_LOG(LogTemp, Log, TEXT("Connect() returned true, checking actual connection state"));

				// Connected 상태로 바로 변경하지 말고, 실제 소켓 상태 확인
				ESocketConnectionState SocketState = ClientSocket->GetConnectionState();
				if (SocketState == SCS_Connected)
				{
					// 정말 즉시 연결된 경우 (매우 드뭄)
					UE_LOG(LogTemp, Log, TEXT("Actually connected immediately"));
					ClientSocket->SetNonBlocking(false);
					SetConnectionState(ENetConnectionState::Connected);
					StopReconnectTimer();
				}
				else
				{
					// 대부분의 경우 - 연결 진행 중
					UE_LOG(LogTemp, Log, TEXT("Connection in progress, starting status check"));
					// 타이머 시작 로직
					if (GetWorld())
					{
						GetWorld()->GetTimerManager().SetTimer(
							ReconnectTimerHandle, this, &ANetworkManager::TryReconnect, 0.1f, true);
					}
				}
			}
			else
			{
				// 논블로킹 연결 진행 중 - 타이머로 상태 체크
				UE_LOG(LogTemp, Log, TEXT("Non-blocking connection started, checking status..."));

				// 기존 재연결 타이머를 연결 상태 체크용으로 활용
				if (GetWorld())
				{
					GetWorld()->GetTimerManager().SetTimer(
						ReconnectTimerHandle,
						this,
						&ANetworkManager::TryReconnect, // 기존 함수를 상태 체크용으로 재활용
						0.1f,  // 100ms마다 체크
						true   // 반복
					);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create server address: %s:%d"), *ServerIP, ServerPort);
			SetConnectionState(ENetConnectionState::Failed);
			CleanupSocket();
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

bool ANetworkManager::LoginToServer(const FString& Username, const FString& Password, FString& OutMessage)
{
	// 기존 유저 데이터 초기화
	ClearUserData();
	OutMessage = TEXT("");

	if (!IsConnectedToServer())
	{
		UE_LOG(LogTemp, Error, TEXT("Not connected to server"));
		OutMessage = TEXT("Not connected to server");
		return false;
	}

	if (!ClientPacketManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientPacketManager is null"));
		OutMessage = TEXT("ClientPacketManager is null");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting login: %s"), *Username);

	// ClientPacketManager를 사용해서 로그인 패킷 생성
	TArray<uint8> LoginPacket = ClientPacketManager->CreateLoginRequest(Username, Password);
	if (LoginPacket.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create login packet: %s"), *ClientPacketManager->GetLastError());
		OutMessage = FString::Printf(TEXT("Failed to create login packet: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	if (!SendPacketData(LoginPacket))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send login packet"));
		OutMessage = TEXT("Failed to send login packet");
		return false;
	}

	// 서버 응답 대기
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		UE_LOG(LogTemp, Error, TEXT("Login response timeout"));
		OutMessage = TEXT("Login response timeout");
		return false;
	}

	// 응답 파싱하여 CurrentUserData에 직접 저장
	if (!ClientPacketManager->ParseLoginResponse(ResponseData, CurrentUserData))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse login response: %s"), *ClientPacketManager->GetLastError());
		OutMessage = FString::Printf(TEXT("Failed to parse login response: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	// 로그인 결과 처리
	if (CurrentUserData.bSuccess)
	{
		OutMessage = CurrentUserData.Message;
		UE_LOG(LogTemp, Log, TEXT("Login Success - UserID: %d, Username: %s, Nickname: %s, Level: %d"),
			CurrentUserData.UserID, *CurrentUserData.Username, *CurrentUserData.Nickname, CurrentUserData.Level);
	}
	else
	{
		OutMessage = CurrentUserData.Message;
		UE_LOG(LogTemp, Warning, TEXT("Login Failed - Message: %s"), *OutMessage);
		// 실패 시 데이터 초기화
		ClearUserData();
	}

	return CurrentUserData.bSuccess;
}

bool ANetworkManager::LogoutFromServer(FString& OutMessage)
{
	if (!IsConnectedToServer())
	{
		UE_LOG(LogTemp, Error, TEXT("Not connected to server"));
		OutMessage = TEXT("Not connected to server");
		return false;
	}

	if (!IsLogin())
	{
		UE_LOG(LogTemp, Warning, TEXT("User is not logged in"));
		OutMessage = TEXT("User is not logged in");
		return false;
	}

	if (!ClientPacketManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientPacketManager is null"));
		OutMessage = TEXT("ClientPacketManager is null");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting logout for UserID: %d"), CurrentUserData.UserID);

	// CurrentUserData에서 UserID 가져와서 로그아웃 패킷 생성
	TArray<uint8> LogoutPacket = ClientPacketManager->CreateLogoutRequest(CurrentUserData.UserID);
	if (LogoutPacket.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create logout packet: %s"), *ClientPacketManager->GetLastError());
		OutMessage = FString::Printf(TEXT("Failed to create logout packet: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	if (!SendPacketData(LogoutPacket))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send logout packet"));
		OutMessage = TEXT("Failed to send logout packet");
		return false;
	}

	// 서버 응답 대기
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		UE_LOG(LogTemp, Error, TEXT("Logout response timeout"));
		OutMessage = TEXT("Logout response timeout");
		return false;
	}

	// 응답 파싱
	if (!ClientPacketManager->ParseLogoutResponse(ResponseData, OutMessage))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse logout response: %s"), *ClientPacketManager->GetLastError());
		OutMessage = FString::Printf(TEXT("Failed to parse logout response: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	// 로그아웃 성공 시 유저 데이터 초기화
	ClearUserData();

	UE_LOG(LogTemp, Log, TEXT("Logout Success - Message: %s"), *OutMessage);
	return true;
}

bool ANetworkManager::CreateAccountToServer(const FString& Username, const FString& Password, const FString& Nickname, int32& OutUserID, FString& OutMessage)
{
	// 초기화
	OutUserID = 0;
	OutMessage = TEXT("");

	if (!IsConnectedToServer())
	{
		UE_LOG(LogTemp, Error, TEXT("Not connected to server"));
		OutMessage = TEXT("Not connected to server");
		return false;
	}

	if (!ClientPacketManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientPacketManager is null"));
		OutMessage = TEXT("ClientPacketManager is null");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting create account: %s (Nickname: %s)"), *Username, *Nickname);

	// ClientPacketManager를 사용해서 회원가입 패킷 생성
	TArray<uint8> CreateAccountPacket = ClientPacketManager->CreateAccountRequest(Username, Password, Nickname);
	if (CreateAccountPacket.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create account packet: %s"), *ClientPacketManager->GetLastError());
		OutMessage = FString::Printf(TEXT("Failed to create account packet: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	if (!SendPacketData(CreateAccountPacket))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send create account packet"));
		OutMessage = TEXT("Failed to send create account packet");
		return false;
	}

	// 서버 응답 대기
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		UE_LOG(LogTemp, Error, TEXT("Create account response timeout"));
		OutMessage = TEXT("Create account response timeout");
		return false;
	}

	bool bSuccess = false;
	if (!ClientPacketManager->ParseCreateAccountResponse(ResponseData, OutUserID, OutMessage, bSuccess))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse create account response: %s"), *ClientPacketManager->GetLastError());
		OutMessage = FString::Printf(TEXT("Failed to parse create account response: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	// 회원가입 결과 로그 출력
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Create Account Success - UserID: %d, Username: %s, Message: %s"),
			OutUserID, *Username, *OutMessage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Create Account Failed - Message: %s"), *OutMessage);
		OutUserID = 0; // 실패 시 UserID를 0으로 설정
	}
	return bSuccess;
}

bool ANetworkManager::CreateGameServer(int32 UserID, const FString& ServerName, const FString& ServerPassword, const FString& ServerIP, int32 ServerPort, int32 MaxPlayers, int32& OutServerID, FString& OutMessage)
{
	// 초기화
	OutServerID = 0;
	OutMessage = TEXT("");

	if (!IsConnectedToServer())
	{
		UE_LOG(LogTemp, Error, TEXT("Not connected to server"));
		OutMessage = TEXT("Not connected to server");
		return false;
	}

	if (!ClientPacketManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientPacketManager is null"));
		OutMessage = TEXT("ClientPacketManager is null");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting create game server: %s"), *ServerName);

	// ClientPacketManager를 사용해서 게임 서버 생성 패킷 생성
	TArray<uint8> CreateServerPacket = ClientPacketManager->CreateGameServerRequest(UserID, ServerName, ServerPassword, ServerIP, ServerPort, MaxPlayers);
	if (CreateServerPacket.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create game server packet: %s"), *ClientPacketManager->GetLastError());
		OutMessage = FString::Printf(TEXT("Failed to create game server packet: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	if (!SendPacketData(CreateServerPacket))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send create game server packet"));
		OutMessage = TEXT("Failed to send create game server packet");
		return false;
	}

	// 서버 응답 대기
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		UE_LOG(LogTemp, Error, TEXT("Create game server response timeout"));
		OutMessage = TEXT("Create game server response timeout");
		return false;
	}

	// ClientPacketManager를 사용해서 응답 파싱
	bool bSuccess = false;
	if (!ClientPacketManager->ParseCreateGameServerResponse(ResponseData, OutServerID, OutMessage, bSuccess))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse create game server response: %s"), *ClientPacketManager->GetLastError());
		OutMessage = FString::Printf(TEXT("Failed to parse create game server response: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	// 게임 서버 생성 결과 로그 출력
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Create Game Server Success - ServerID: %d, Name: %s"), OutServerID, *ServerName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Create Game Server Failed - Message: %s"), *OutMessage);
		OutServerID = 0;
	}

	return bSuccess;
}

bool ANetworkManager::GetGameServerList(TArray<FAccountGameServerInfo>& OutServerList)
{
	// 초기화
	OutServerList.Empty();

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

	UE_LOG(LogTemp, Log, TEXT("Requesting game server list"));

	// ClientPacketManager를 사용해서 게임 서버 리스트 요청 패킷 생성 (RequestType = 0)
	TArray<uint8> ServerListPacket = ClientPacketManager->CreateGameServerListRequest(0);
	if (ServerListPacket.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create game server list packet: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	if (!SendPacketData(ServerListPacket))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send game server list packet"));
		return false;
	}

	// 서버 응답 대기
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		UE_LOG(LogTemp, Error, TEXT("Game server list response timeout"));
		return false;
	}

	// ClientPacketManager를 사용해서 응답 파싱
	if (!ClientPacketManager->ParseGameServerListResponse(ResponseData, OutServerList))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse game server list response: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Game server list received: %d servers"), OutServerList.Num());
	return true;
}

bool ANetworkManager::JoinGameServer(int32 UserID, int32 ServerID, const FString& ServerPassword, FString& OutServerIP, int32& OutServerPort, FString& OutMessage)
{
	// 초기화
	OutServerIP = TEXT("");
	OutServerPort = 0;
	OutMessage = TEXT("");

	if (!IsConnectedToServer())
	{
		UE_LOG(LogTemp, Error, TEXT("Not connected to server"));
		OutMessage = TEXT("Not connected to server");
		return false;
	}

	if (!ClientPacketManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientPacketManager is null"));
		OutMessage = TEXT("ClientPacketManager is null");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting join game server: ServerID %d"), ServerID);

	// ClientPacketManager를 사용해서 게임 서버 접속 패킷 생성
	TArray<uint8> JoinServerPacket = ClientPacketManager->CreateJoinGameServerRequest(UserID, ServerID, ServerPassword);
	if (JoinServerPacket.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create join game server packet: %s"), *ClientPacketManager->GetLastError());
		OutMessage = FString::Printf(TEXT("Failed to create join game server packet: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	if (!SendPacketData(JoinServerPacket))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send join game server packet"));
		OutMessage = TEXT("Failed to send join game server packet");
		return false;
	}

	// 서버 응답 대기
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		UE_LOG(LogTemp, Error, TEXT("Join game server response timeout"));
		OutMessage = TEXT("Join game server response timeout");
		return false;
	}

	// ClientPacketManager를 사용해서 응답 파싱
	bool bSuccess = false;
	if (!ClientPacketManager->ParseJoinGameServerResponse(ResponseData, OutServerIP, OutServerPort, OutMessage, bSuccess))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse join game server response: %s"), *ClientPacketManager->GetLastError());
		OutMessage = FString::Printf(TEXT("Failed to parse join game server response: %s"), *ClientPacketManager->GetLastError());
		return false;
	}

	// 게임 서버 접속 결과 로그 출력
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Join Game Server Success - IP: %s, Port: %d"), *OutServerIP, OutServerPort);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Join Game Server Failed - Message: %s"), *OutMessage);
		OutServerIP = TEXT("");
		OutServerPort = 0;
	}

	return bSuccess;
}

bool ANetworkManager::GetPlayerData(int32 UserID, FAccountPlayerData& OutPlayerData, FString& OutMessage)
{
	OutPlayerData = FAccountPlayerData();
	OutMessage = TEXT("");

	if (!IsConnectedToServer() || !ClientPacketManager)
	{
		OutMessage = TEXT("서버 연결 또는 PacketManager 오류");
		return false;
	}

	// 플레이어 데이터 요청 (RequestType = 0: 조회)
	TArray<uint8> Packet = ClientPacketManager->CreatePlayerDataRequest(UserID);
	if (Packet.Num() == 0 || !SendPacketData(Packet))
	{
		OutMessage = TEXT("플레이어 데이터 요청 실패");
		return false;
	}

	// 응답 수신
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		OutMessage = TEXT("플레이어 데이터 응답 시간 초과");
		return false;
	}

	// 응답 파싱
	if (!ClientPacketManager->ParsePlayerDataResponse(ResponseData, OutPlayerData))
	{
		OutMessage = TEXT("플레이어 데이터 파싱 실패");
		return false;
	}

	OutMessage = TEXT("플레이어 데이터 조회 성공");
	return true;
}

bool ANetworkManager::GetPlayerInventory(int32 UserID, TArray<FAccountItemInfo>& OutInventoryItems, FString& OutMessage)
{
	OutInventoryItems.Empty();
	OutMessage = TEXT("");

	if (!IsConnectedToServer() || !ClientPacketManager)
	{
		OutMessage = TEXT("서버 연결 또는 PacketManager 오류");
		return false;
	}

	// 인벤토리 조회 (RequestType = 0: 인벤토리, ItemID = 0: 전체)
	TArray<uint8> Packet = ClientPacketManager->CreateItemDataRequest(UserID, 0, 0, 0);
	if (Packet.Num() == 0 || !SendPacketData(Packet))
	{
		OutMessage = TEXT("인벤토리 요청 실패");
		return false;
	}

	// 응답 수신 및 파싱
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		OutMessage = TEXT("인벤토리 응답 시간 초과");
		return false;
	}

	if (!ClientPacketManager->ParseItemDataResponse(ResponseData, OutInventoryItems))
	{
		OutMessage = TEXT("인벤토리 파싱 실패");
		return false;
	}

	OutMessage = FString::Printf(TEXT("인벤토리 조회 성공 (%d개 아이템)"), OutInventoryItems.Num());
	return true;
}

bool ANetworkManager::GetStoreItemList(int32 ShopID, TArray<FAccountItemInfo>& OutShopItems, FString& OutMessage)
{
	OutShopItems.Empty();
	OutMessage = TEXT("");

	if (!IsConnectedToServer() || !ClientPacketManager)
	{
		OutMessage = TEXT("서버 연결 또는 PacketManager 오류");
		return false;
	}

	// 상점 아이템 목록 요청
	TArray<uint8> Packet = ClientPacketManager->CreateStoreItemsRequest(ShopID);
	if (Packet.Num() == 0 || !SendPacketData(Packet))
	{
		OutMessage = TEXT("상점 아이템 요청 실패");
		return false;
	}

	// 응답 수신 및 파싱
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		OutMessage = TEXT("상점 아이템 응답 시간 초과");
		return false;
	}

	if (!ClientPacketManager->ParseStoreItemsResponse(ResponseData, OutShopItems))
	{
		OutMessage = TEXT("상점 아이템 파싱 실패");
		return false;
	}

	OutMessage = FString::Printf(TEXT("상점 아이템 조회 성공 (%d개 아이템)"), OutShopItems.Num());
	return true;
}

bool ANetworkManager::PurchaseItem(int32 UserID, int32 ItemID, int32 ItemCount, int32& OutNewGold, FString& OutMessage)
{
	OutNewGold = 0;
	OutMessage = TEXT("");

	if (!IsConnectedToServer() || !ClientPacketManager)
	{
		OutMessage = TEXT("서버 연결 또는 PacketManager 오류");
		return false;
	}

	if (UserID <= 0 || ItemID <= 0 || ItemCount <= 0)
	{
		OutMessage = TEXT("유효하지 않은 구매 정보입니다");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting to purchase item: UserID %d, ItemID %d, Count %d"),
		UserID, ItemID, ItemCount);

	// 구매 요청 패킷 생성 (TransactionType = 0: 구매)
	TArray<uint8> Packet = ClientPacketManager->CreateShopTransactionRequest(UserID, ItemID, ItemCount, 0);
	if (Packet.Num() == 0 || !SendPacketData(Packet))
	{
		OutMessage = TEXT("아이템 구매 요청 실패");
		return false;
	}

	// 응답 수신 및 파싱
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		OutMessage = TEXT("아이템 구매 응답 시간 초과");
		return false;
	}

	bool bSuccess = false;
	if (!ClientPacketManager->ParseShopTransactionResponse(ResponseData, OutNewGold, OutMessage, bSuccess))
	{
		OutMessage = TEXT("아이템 구매 응답 파싱 실패");
		return false;
	}

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Item purchase successful: NewGold %d"), OutNewGold);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Item purchase failed: %s"), *OutMessage);
	}

	return bSuccess;
}

bool ANetworkManager::SellItem(int32 UserID, int32 ItemID, int32 ItemCount, int32& OutNewGold, FString& OutMessage)
{
	OutNewGold = 0;
	OutMessage = TEXT("");

	if (!IsConnectedToServer() || !ClientPacketManager)
	{
		OutMessage = TEXT("서버 연결 또는 PacketManager 오류");
		return false;
	}

	if (UserID <= 0 || ItemID <= 0 || ItemCount <= 0)
	{
		OutMessage = TEXT("유효하지 않은 판매 정보입니다");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting to sell item: UserID %d, ItemID %d, Count %d"),
		UserID, ItemID, ItemCount);

	// 판매 요청 패킷 생성 (TransactionType = 1: 판매)
	TArray<uint8> Packet = ClientPacketManager->CreateShopTransactionRequest(UserID, ItemID, ItemCount, 1);
	if (Packet.Num() == 0 || !SendPacketData(Packet))
	{
		OutMessage = TEXT("아이템 판매 요청 실패");
		return false;
	}

	// 응답 수신 및 파싱
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		OutMessage = TEXT("아이템 판매 응답 시간 초과");
		return false;
	}

	bool bSuccess = false;
	if (!ClientPacketManager->ParseShopTransactionResponse(ResponseData, OutNewGold, OutMessage, bSuccess))
	{
		OutMessage = TEXT("아이템 판매 응답 파싱 실패");
		return false;
	}

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Item sell successful: NewGold %d"), OutNewGold);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Item sell failed: %s"), *OutMessage);
	}

	return bSuccess;
}

bool ANetworkManager::GetSingleItemInfo(int32 UserID, int32 ItemID, FAccountItemInfo& OutItemInfo, FString& OutMessage)
{
	OutItemInfo = FAccountItemInfo();
	OutMessage = TEXT("");

	if (!IsConnectedToServer() || !ClientPacketManager)
	{
		OutMessage = TEXT("서버 연결 또는 PacketManager 오류");
		return false;
	}

	if (UserID <= 0)
	{
		OutMessage = TEXT("유효하지 않은 사용자 ID입니다");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Requesting single item info: UserID %d, ItemID %d"), UserID, ItemID);

	// 특정 아이템 정보 요청 (RequestType = 3: 특정 아이템 조회)
	TArray<uint8> Packet = ClientPacketManager->CreateItemDataRequest(UserID, 3, ItemID, 0);
	if (Packet.Num() == 0 || !SendPacketData(Packet))
	{
		OutMessage = TEXT("아이템 정보 요청 실패");
		return false;
	}

	// 응답 수신 및 파싱
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		OutMessage = TEXT("아이템 정보 응답 시간 초과");
		return false;
	}

	TArray<FAccountItemInfo> ItemList;
	if (!ClientPacketManager->ParseItemDataResponse(ResponseData, ItemList))
	{
		OutMessage = TEXT("아이템 정보 파싱 실패");
		return false;
	}

	if (ItemList.Num() > 0)
	{
		OutItemInfo = ItemList[0];
		UE_LOG(LogTemp, Log, TEXT("Single item info received: %s (ID: %d)"), *OutItemInfo.ItemName, OutItemInfo.ItemID);
		OutMessage = TEXT("아이템 정보 조회 성공");
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Item not found: ItemID %d"), ItemID);
		OutMessage = TEXT("아이템을 찾을 수 없습니다");
		return false;
	}
}

bool ANetworkManager::RemoveItemFromInventory(int32 UserID, int32 ItemID, int32 ItemCount, FString& OutMessage)
{
	OutMessage = TEXT("");

	if (!IsConnectedToServer() || !ClientPacketManager)
	{
		OutMessage = TEXT("서버 연결 또는 PacketManager 오류");
		return false;
	}

	if (UserID <= 0)
	{
		OutMessage = TEXT("유효하지 않은 사용자 ID입니다");
		return false;
	}

	if (ItemID <= 0)
	{
		OutMessage = TEXT("유효하지 않은 아이템 ID입니다");
		return false;
	}

	if (ItemCount <= 0)
	{
		OutMessage = TEXT("제거할 아이템 수량은 1개 이상이어야 합니다");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting to remove item from inventory: UserID %d, ItemID %d, Count %d"),
		UserID, ItemID, ItemCount);

	// 아이템 제거 요청 패킷 생성 (RequestType = 2: 아이템 제거)
	TArray<uint8> Packet = ClientPacketManager->CreateItemDataRequest(UserID, 2, ItemID, ItemCount);
	if (Packet.Num() == 0 || !SendPacketData(Packet))
	{
		OutMessage = TEXT("아이템 제거 요청 실패");
		return false;
	}

	// 응답 수신 및 파싱
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		OutMessage = TEXT("아이템 제거 응답 시간 초과");
		return false;
	}

	// 아이템 데이터 응답 파싱 (실제로는 성공/실패만 확인)
	TArray<FAccountItemInfo> DummyItems; // 제거 응답에서는 사용하지 않음
	if (!ClientPacketManager->ParseItemDataResponse(ResponseData, DummyItems))
	{
		OutMessage = TEXT("아이템 제거 응답 파싱 실패");
		return false;
	}

	OutMessage = FString::Printf(TEXT("인벤토리에서 아이템 제거 성공: 아이템 ID %d, 수량 %d개"), ItemID, ItemCount);
	UE_LOG(LogTemp, Log, TEXT("Item removal successful: ItemID %d, Count %d"), ItemID, ItemCount);

	return true;
}

bool ANetworkManager::AddItemToInventory(int32 UserID, int32 ItemID, int32 ItemCount, FString& OutMessage)
{
	OutMessage = TEXT("");

	if (!IsConnectedToServer() || !ClientPacketManager)
	{
		OutMessage = TEXT("서버 연결 또는 PacketManager 오류");
		return false;
	}

	if (UserID <= 0)
	{
		OutMessage = TEXT("유효하지 않은 사용자 ID입니다");
		return false;
	}

	if (ItemID <= 0)
	{
		OutMessage = TEXT("유효하지 않은 아이템 ID입니다");
		return false;
	}

	if (ItemCount <= 0)
	{
		OutMessage = TEXT("추가할 아이템 수량은 1개 이상이어야 합니다");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Adding item to inventory: UserID %d, ItemID %d, Count %d"), UserID, ItemID, ItemCount);

	// 아이템 추가 요청 (RequestType = 1: 아이템 추가)
	TArray<uint8> Packet = ClientPacketManager->CreateItemDataRequest(UserID, 1, ItemID, ItemCount);
	if (Packet.Num() == 0 || !SendPacketData(Packet))
	{
		OutMessage = TEXT("아이템 추가 요청 실패");
		return false;
	}

	// 응답 수신 및 파싱
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		OutMessage = TEXT("아이템 추가 응답 시간 초과");
		return false;
	}

	// 응답 파싱 (아이템 데이터 응답으로 파싱)
	TArray<FAccountItemInfo> DummyItems;
	if (!ClientPacketManager->ParseItemDataResponse(ResponseData, DummyItems))
	{
		OutMessage = TEXT("아이템 추가 응답 파싱 실패");
		return false;
	}

	OutMessage = FString::Printf(TEXT("아이템 추가 성공: %d개"), ItemCount);
	UE_LOG(LogTemp, Log, TEXT("Item added successfully: ItemID %d, Count %d"), ItemID, ItemCount);
	return true;
}

bool ANetworkManager::SetItemQuantity(int32 UserID, int32 ItemID, int32 NewQuantity, FString& OutMessage)
{
	OutMessage = TEXT("");

	if (!IsConnectedToServer() || !ClientPacketManager)
	{
		OutMessage = TEXT("서버 연결 또는 PacketManager 오류");
		return false;
	}

	if (UserID <= 0)
	{
		OutMessage = TEXT("유효하지 않은 사용자 ID입니다");
		return false;
	}

	if (ItemID <= 0)
	{
		OutMessage = TEXT("유효하지 않은 아이템 ID입니다");
		return false;
	}

	if (NewQuantity < 0)
	{
		OutMessage = TEXT("아이템 수량은 0개 이상이어야 합니다");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Setting item quantity: UserID %d, ItemID %d, NewQuantity %d"), UserID, ItemID, NewQuantity);

	// 1단계: 현재 인벤토리 조회
	TArray<FAccountItemInfo> InventoryItems;
	FString InventoryMessage;
	if (!GetPlayerInventory(UserID, InventoryItems, InventoryMessage))
	{
		OutMessage = FString::Printf(TEXT("인벤토리 조회 실패: %s"), *InventoryMessage);
		return false;
	}

	// 2단계: 현재 해당 아이템의 수량 찾기
	int32 CurrentQuantity = 0;
	bool bItemFound = false;

	for (const FAccountItemInfo& Item : InventoryItems)
	{
		if (Item.ItemID == ItemID)
		{
			CurrentQuantity = Item.Quantity;
			bItemFound = true;
			break;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Current quantity: %d, Target quantity: %d"), CurrentQuantity, NewQuantity);

	// 3단계: 수량 차이 계산 및 처리
	if (CurrentQuantity == NewQuantity)
	{
		OutMessage = TEXT("이미 목표 수량과 동일합니다");
		return true;
	}
	else if (NewQuantity > CurrentQuantity)
	{
		// 수량 증가 - 아이템 추가
		int32 AddCount = NewQuantity - CurrentQuantity;
		UE_LOG(LogTemp, Log, TEXT("Adding %d items"), AddCount);

		TArray<uint8> Packet = ClientPacketManager->CreateItemDataRequest(UserID, 1, ItemID, AddCount);
		if (Packet.Num() == 0 || !SendPacketData(Packet))
		{
			OutMessage = TEXT("아이템 추가 요청 실패");
			return false;
		}
	}
	else
	{
		// 수량 감소 - 아이템 제거
		int32 RemoveCount = CurrentQuantity - NewQuantity;
		UE_LOG(LogTemp, Log, TEXT("Removing %d items"), RemoveCount);

		TArray<uint8> Packet = ClientPacketManager->CreateItemDataRequest(UserID, 2, ItemID, RemoveCount);
		if (Packet.Num() == 0 || !SendPacketData(Packet))
		{
			OutMessage = TEXT("아이템 제거 요청 실패");
			return false;
		}
	}

	// 4단계: 응답 수신 및 파싱
	TArray<uint8> ResponseData;
	if (!ReceivePacketData(ResponseData, PacketTimeout))
	{
		OutMessage = TEXT("아이템 수량 변경 응답 시간 초과");
		return false;
	}

	TArray<FAccountItemInfo> DummyItems;
	if (!ClientPacketManager->ParseItemDataResponse(ResponseData, DummyItems))
	{
		OutMessage = TEXT("아이템 수량 변경 응답 파싱 실패");
		return false;
	}

	OutMessage = FString::Printf(TEXT("아이템 수량 설정 성공: %d → %d개"), CurrentQuantity, NewQuantity);
	UE_LOG(LogTemp, Log, TEXT("Item quantity set successfully: ItemID %d, %d -> %d"), ItemID, CurrentQuantity, NewQuantity);
	return true;
}

bool ANetworkManager::IsLogin() const
{
	return CurrentUserData.bSuccess;
}

int32 ANetworkManager::GetCurrentUserID() const
{
	return CurrentUserData.UserID;
}

FString ANetworkManager::GetCurrentUsername() const
{
	return CurrentUserData.Username;
}

FString ANetworkManager::GetCurrentNickname() const
{
	return CurrentUserData.Nickname;
}

int32 ANetworkManager::GetCurrentLevel() const
{
	return CurrentUserData.Level;
}

FAccountLoginResponse ANetworkManager::GetCurrentUserData() const
{
	return CurrentUserData;
}

void ANetworkManager::ClearUserData()
{
	CurrentUserData = FAccountLoginResponse(); // 구조체 기본값으로 초기화
	UE_LOG(LogTemp, Log, TEXT("User data cleared"));
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
		// 재연결 간격을 더 자주 체크하도록 수정 (상태 체크용으로도 사용)
		float CheckInterval = (ConnectionState == ENetConnectionState::Connecting) ? 0.1f : ReconnectInterval;

		GetWorld()->GetTimerManager().SetTimer(
			ReconnectTimerHandle,
			this,
			&ANetworkManager::TryReconnect,
			CheckInterval,
			true // 반복
		);

		UE_LOG(LogTemp, Log, TEXT("Timer started (%.1f seconds interval)"), CheckInterval);
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
	// 현재 연결 상태가 Connecting인 경우 - 연결 진행 상태 체크
	if (ConnectionState == ENetConnectionState::Connecting && ClientSocket)
	{
		ESocketConnectionState SocketState = ClientSocket->GetConnectionState();

		if (SocketState == SCS_Connected)
		{
			// 연결 성공
			UE_LOG(LogTemp, Log, TEXT("Async connection successful!"));
			StopReconnectTimer();
			ClientSocket->SetNonBlocking(false); // 블로킹 모드로 복원
			SetConnectionState(ENetConnectionState::Connected);
			return;
		}
		else if (SocketState == SCS_ConnectionError)
		{
			// 연결 실패 - 재연결 모드로 전환
			UE_LOG(LogTemp, Warning, TEXT("Connection failed - switching to reconnect mode"));
			CleanupSocket();
			SetConnectionState(ENetConnectionState::Reconnecting);
			return;
		}
		// SCS_NotConnected인 경우 계속 대기 (연결 진행 중)
		return;
	}

	// 기존 재연결 로직 (Reconnecting 상태일 때)
	if (ConnectionState != ENetConnectionState::Reconnecting)
	{
		StopReconnectTimer();
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting to reconnect to server: %s:%d"), *CurrentServerIP, CurrentServerPort);

	CleanupSocket();

	if (CreateSocket())
	{
		TSharedRef<FInternetAddr> ServerAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

		if (CreateServerAddress(CurrentServerIP, CurrentServerPort, ServerAddress))
		{
			// 재연결도 논블로킹으로 시도
			ClientSocket->SetNonBlocking(true);
			bool bConnected = ClientSocket->Connect(*ServerAddress);

			if (bConnected)
			{
				UE_LOG(LogTemp, Log, TEXT("Reconnection successful!"));
				StopReconnectTimer();
				ClientSocket->SetNonBlocking(false);
				SetConnectionState(ENetConnectionState::Connected);
			}
			else
			{
				// 재연결 진행 중 - 상태를 Connecting으로 변경하여 상태 체크 모드로 전환
				SetConnectionState(ENetConnectionState::Connecting);
				// 타이머는 그대로 유지 (이미 실행 중)
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create server address during reconnection"));
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

bool ANetworkManager::CreateServerAddress(const FString& IP, int32 Port, TSharedRef<FInternetAddr>& OutAddress)
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get socket subsystem"));
		return false;
	}

	// 방법 1: FIPv4Address + FIPv4Endpoint만 사용 (SetPort 완전 회피)
	FIPv4Address IPv4Address;
	if (FIPv4Address::Parse(IP, IPv4Address))
	{
		FIPv4Endpoint ServerEndpoint(IPv4Address, static_cast<uint16>(Port));
		OutAddress = ServerEndpoint.ToInternetAddr();
		UE_LOG(LogTemp, Warning, TEXT("Address created using IPv4Endpoint: %s:%d"), *IP, Port);
		return true;
	}

	// 방법 2: localhost 특별 처리
	FString CleanIP = IP.TrimStartAndEnd();
	if (CleanIP.Equals(TEXT("localhost"), ESearchCase::IgnoreCase))
	{
		FIPv4Address LocalhostAddr;
		if (FIPv4Address::Parse(TEXT("127.0.0.1"), LocalhostAddr))
		{
			FIPv4Endpoint LocalhostEndpoint(LocalhostAddr, static_cast<uint16>(Port));
			OutAddress = LocalhostEndpoint.ToInternetAddr();
			UE_LOG(LogTemp, Log, TEXT("Address created for localhost: %s:%d"), *IP, Port);
			return true;
		}
	}

	// 방법 3: 도메인 주소 해석
	if (!IP.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempting DNS resolution for: %s"), *CleanIP);

		TSharedRef<FInternetAddr> TempAddr = SocketSubsystem->CreateInternetAddr();
		ESocketErrors Result = SocketSubsystem->GetHostByName(
			TCHAR_TO_ANSI(*CleanIP),
			*TempAddr
		);

		UE_LOG(LogTemp, Warning, TEXT("DNS Result: %d"), (int32)Result);
		UE_LOG(LogTemp, Warning, TEXT("Resolved address before SetPort: %s"), *TempAddr->ToString(true));

		if (Result == SE_NO_ERROR)
		{
			// 포트 설정 전 주소 확인
			FString ResolvedIP = TempAddr->ToString(false);
			UE_LOG(LogTemp, Warning, TEXT("✓ DNS resolved %s to IP: %s"), *CleanIP, *ResolvedIP);

			TempAddr->SetPort(Port);
			OutAddress = TempAddr;

			UE_LOG(LogTemp, Warning, TEXT("✓ Final domain address: %s"), *OutAddress->ToString(true));
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("✗ DNS resolution failed for %s (Error: %d)"),
				*CleanIP, (int32)Result);

			// 추가 디버깅: 다른 방법으로도 시도
			UE_LOG(LogTemp, Warning, TEXT("Trying alternative method..."));
			TSharedRef<FInternetAddr> AltAddr = SocketSubsystem->CreateInternetAddr();
			bool bIsValid = false;
			AltAddr->SetIp(*CleanIP, bIsValid);

			if (bIsValid)
			{
				AltAddr->SetPort(Port);
				OutAddress = AltAddr;
				UE_LOG(LogTemp, Log, TEXT("✓ Alternative method succeeded: %s"), *OutAddress->ToString(true));
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("✗ Alternative method also failed"));
			}
		}
	}
	// 실패 시

	UE_LOG(LogTemp, Error, TEXT("Failed to create address for: %s:%d"), *IP, Port);
	return false;
}
