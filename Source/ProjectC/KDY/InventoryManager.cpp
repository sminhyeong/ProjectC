// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManager.h"
#include "InventoryWidget.h"

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

bool AInventoryManager::C2S_AddItem_Validate(FRPGItemData NewItemData)
{
	// 인벤토리가 가득 찼는지 확인
	TArray<FRPGItemData> CheckList;
	switch (NewItemData.ItemInfo.Category)
	{
		case EItemCategory::WEAPON:
		{
			CheckList = WeaponList;
			break;
		}
		case EItemCategory::ARMOR:
		{
			CheckList = ArmorList;
			break;
		}
		case EItemCategory::CONSUME:
		{
			CheckList = ConsumeList;
			break;
		}
		default:
			break;
	}
	if (CheckList.Num() >= MaxItemPerCategory)
	{
		InventoryIsFull();
		return false;
	}
	else
	{
		return true;
	}

	return false;
}

void AInventoryManager::C2S_AddItem_Implementation(FRPGItemData NewItemData)
{
	switch (NewItemData.ItemInfo.Category)
	{
		case EItemCategory::WEAPON:
		{
			WeaponList.Add(NewItemData);
			break;
		}
		case EItemCategory::ARMOR:
		{
			ArmorList.Add(NewItemData);
			break;
		}
		case EItemCategory::CONSUME:
		{
			ConsumeList.Add(NewItemData);
			break;
		}
		default:
			break;
	}
	if (NewItemData.ItemInfo.Category == InventoryWidget->NowWatchCategory)
	{
		S2C_UpdateInventory(NewItemData.ItemInfo.Category);
	}
}

void AInventoryManager::S2C_UpdateInventory_Implementation(EItemCategory UpdateListCategory)
{
	switch (UpdateListCategory)
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

