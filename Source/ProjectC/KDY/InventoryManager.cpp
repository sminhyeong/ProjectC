// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManager.h"
#include "InventoryWidget.h"
#include "../SMH/NetGameInstanceBase.h"
#include "../SMH/NetworkManager.h"

// Sets default values
AInventoryManager::AInventoryManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

bool AInventoryManager::TryAddItem(int32 NewItemID, int32 NewItemCount)
{
	UNetGameInstanceBase* NetGameInstance = Cast<UNetGameInstanceBase>(GetGameInstance());
	if (!NetGameInstance)
	{
		return false;
	}
	UNetworkManager* NetworkManager = NetGameInstance->GetNetworkManager();
	if (!NetworkManager)
	{
		return false;
	}

	// 현재 아이템 정보 가져오기
	FAccountLoginResponse UserData = NetworkManager->GetCurrentUserData();
	FAccountItemInfo NewItemInfo;
	FString OutMessage;
	if (NetworkManager->GetSingleItemInfo(UserData.UserID, NewItemID, NewItemInfo, OutMessage))
	{
		EItemCategory NewItemCategory = ItemStruct::ConvertTypeToCategory(NewItemInfo.ItemType);
		// 인벤토리에 자리가 있을 때
		if (CheckInventoryHasSpace(NewItemCategory))
		{
			// 아이템 추가
			if (NetworkManager->AddItemToInventory(UserData.UserID, NewItemID, NewItemCount, OutMessage))
			{
				//// 인벤토리 위젯을 위한 정보 갱신
				//UpdateInventoryList();
				//if (NewItemCategory == InventoryWidget->NowWatchCategory)
				//{
					UpdateInventoryWidget();
				//}
				return true;
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
	return false;
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

void AInventoryManager::UpdateInventoryList()
{
	UNetGameInstanceBase* NetGameInstance = Cast<UNetGameInstanceBase>(GetGameInstance());
	if (!NetGameInstance)
	{
		return;
	}
	UNetworkManager* NetworkManager = NetGameInstance->GetNetworkManager();
	if (!NetworkManager)
	{
		return;
	}
	FAccountLoginResponse UserData = NetworkManager->GetCurrentUserData();

	// AccountItemInfo 정보 불러오기
	TArray<FAccountItemInfo> InventoryItems;
	FString OutMessage;
	if (!NetworkManager->GetPlayerInventory(UserData.UserID, InventoryItems, OutMessage))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *OutMessage);
		return;
	}

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

	for (auto item : InventoryItems)
	{
		FItemAllInfo AllItemData;
		AllItemData.AccountItemInfo = item;

		FString ItemIDString = FString::FromInt(item.ItemID);
		FItemArtInfo* ItemArtDTInfo = DT_ItemArtInfo->FindRow<FItemArtInfo>(FName(*ItemIDString), ItemIDString);
		AllItemData.ItemArtInfo = *ItemArtDTInfo;

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

void AInventoryManager::UpdateInventoryWidget()
{
	if (false) return;

	switch (InventoryWidget->NowWatchCategory)
	{
	case EItemCategory::WEAPON:
	{
		InventoryWidget->UpdateItemList(WeaponList);
		break;
	}
	case EItemCategory::ARMOR:
	{
		InventoryWidget->UpdateItemList(ArmorList);
		break;
	}
	case EItemCategory::CONSUME:
	{
		InventoryWidget->UpdateItemList(ConsumeList);
		break;
	}
	default:
		break;
	}
}

void AInventoryManager::InventoryIsFull_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Inventory is Full"));
}

