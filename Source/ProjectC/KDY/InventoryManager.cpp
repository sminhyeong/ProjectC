// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManager.h"
#include "InventoryWidget.h"
#include "../SMH/NetGameInstanceBase.h"
#include "../SMH/NetworkManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AInventoryManager::AInventoryManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AInventoryManager::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AInventoryManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInventoryManager::C2S_TryAddItem_Implementation(int32 NewItemID, int32 NewItemCount, int32 UserID)
{
	NetworkManager = GetNetworkManager_Inventory();
	if (!NetworkManager)
	{
		return;
	}

	// 더할 아이템 정보 가져오기
	FAccountItemInfo NewItemInfo;
	FString OutMessage;
	// 카테고리 정보가 필요해서 ItemInfo를 받아옴
	if (NetworkManager->GetSingleItemInfo(UserID, NewItemID, NewItemInfo, OutMessage))
	{
		EItemCategory NewItemCategory = ItemStruct::ConvertTypeToCategory(NewItemInfo.ItemType);
		// 인벤토리에 자리가 있을 때
		if (CheckInventoryHasSpace(NewItemCategory))
		{
			// 아이템 추가
			if (NetworkManager->AddItemToInventory(UserID, NewItemID, NewItemCount, OutMessage))
			{
				// 인벤토리 정보 갱신
				UpdateInventoryData(UserID);
				if (NewItemCategory == InventoryWindowWidget->NowWatchCategory)
				{
					S2C_UpdateInventoryWidget();
				}
				UE_LOG(LogTemp, Warning, TEXT("Success Add Item"));
				return;
			}
			else
			{
				// 아이템 추가 실패
				UE_LOG(LogTemp, Warning, TEXT("%s"), *OutMessage);
			}
		}
		else
		{
			InventoryIsFull();
		}
	}
	else
	{
		// 아이템 정보 가져오기 실패
		UE_LOG(LogTemp, Warning, TEXT("%s"), *OutMessage);
	}
	return;
}

void AInventoryManager::C2S_TrySubtractItem_Implementation(int32 SubItemID, int32 SubItemCount, int32 UserID)
{
	NetworkManager = GetNetworkManager_Inventory();
	if (!NetworkManager)
	{
		return;
	}

	// 뺄 아이템 정보 가져오기
	FAccountItemInfo SubItemInfo;
	FString OutMessage;
	// 카테고리 정보가 필요해서 ItemInfo를 받아옴
	if (NetworkManager->GetSingleItemInfo(UserID, SubItemID, SubItemInfo, OutMessage))
	{
		EItemCategory SubItemCategory = ItemStruct::ConvertTypeToCategory(SubItemInfo.ItemType);
		// 아이템 제거
		if (NetworkManager->RemoveItemFromInventory(UserID, SubItemID, SubItemCount, OutMessage))
		{
			// 인벤토리 정보 갱신
			UpdateInventoryData(UserID);
			if (SubItemCategory == InventoryWindowWidget->NowWatchCategory)
			{
				S2C_UpdateInventoryWidget();
			}
			UE_LOG(LogTemp, Warning, TEXT("Success Sub Item"));
			return;
		}
		else
		{
			// 아이템 제거 실패
			UE_LOG(LogTemp, Warning, TEXT("%s"), *OutMessage);
		}
	}
	else
	{
		// 아이템 정보 가져오기 실패
		UE_LOG(LogTemp, Warning, TEXT("%s"), *OutMessage);
	}
	return;
}

bool AInventoryManager::CheckInventoryHasSpace(EItemCategory ItemCategory)
{
	switch (ItemCategory)
	{
	case EItemCategory::WEAPON:
	{
		return WeaponList.Num() < MaxItemPerCategory;
	}
	case EItemCategory::ARMOR:
	{
		return ArmorList.Num() < MaxItemPerCategory;
	}
	case EItemCategory::CONSUME:
	{
		return ConsumeList.Num() < MaxItemPerCategory;
	}
	default:
		return false;
	}
}

void AInventoryManager::UpdateInventoryData(int32 UseId)
{
	S2C_ParseInventoryData(GetInventoryDataToDB(UseId));
}

void AInventoryManager::S2C_UpdateInventoryWidget_Implementation()
{
	switch (InventoryWindowWidget->NowWatchCategory)
	{
	case EItemCategory::WEAPON:
	{
		InventoryWindowWidget->UpdateItemList(WeaponList);
		break;
	}
	case EItemCategory::ARMOR:
	{
		InventoryWindowWidget->UpdateItemList(ArmorList);
		break;
	}
	case EItemCategory::CONSUME:
	{
		InventoryWindowWidget->UpdateItemList(ConsumeList);
		break;
	}
	default:
	{
		break;
	}
	}
}

void AInventoryManager::InventoryIsFull_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Inventory is Full"));
}

void AInventoryManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInventoryManager, WeaponList);
	DOREPLIFETIME(AInventoryManager, ArmorList);
	DOREPLIFETIME(AInventoryManager, ConsumeList);
}

TArray<FAccountItemInfo> AInventoryManager::GetInventoryDataToDB(int32 UseId)
{
	NetworkManager = GetNetworkManager_Inventory();
	if (!NetworkManager)
	{
		return TArray<FAccountItemInfo>();
	}
	FAccountLoginResponse UserData = NetworkManager->GetCurrentUserData();

	// AccountItemInfo 정보 불러오기
	TArray<FAccountItemInfo> InventoryItems;
	FString OutMessage;
	if (!NetworkManager->GetPlayerInventory(UseId, InventoryItems, OutMessage))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *OutMessage);
		return TArray<FAccountItemInfo>();
	}
	return InventoryItems;
}

void AInventoryManager::S2C_ParseInventoryData_Implementation(const TArray<FAccountItemInfo>& InventoryItems)
{
	// ItemArtInfo 정보 불러오기
	if (!DT_ItemArtInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("No ItemArtInfo Data Table"));
		return;
	}
	if (DT_ItemArtInfo->GetRowMap().Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Data Table is Not Valid"));
		return;
	}

	WeaponList.Reset();
	ArmorList.Reset();
	ConsumeList.Reset();
	for (auto item : InventoryItems)
	{
		FItemAllInfo AllItemData;
		AllItemData.AccountItemInfo = item;

		FString ItemIDString = FString::FromInt(item.ItemID);
		FItemArtInfo* ItemArtDTInfo = DT_ItemArtInfo->FindRow<FItemArtInfo>(FName(*ItemIDString), ItemIDString);
		if (ItemArtDTInfo)
		{
			AllItemData.ItemArtInfo = *ItemArtDTInfo;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No Item Art Info"));
			return;
		}

		switch (ItemStruct::ConvertTypeToCategory(item.ItemType))
		{
		case EItemCategory::WEAPON:
		{
			WeaponList.Add(AllItemData);
			break;
		}
		case EItemCategory::ARMOR:
		{
			ArmorList.Add(AllItemData);
			break;
		}
		case EItemCategory::CONSUME:
		{
			ConsumeList.Add(AllItemData);
			break;
		}
		default:
		{
			UE_LOG(LogTemp, Warning, TEXT("Wrong Category"));
			break;
		}
		}
	}
}