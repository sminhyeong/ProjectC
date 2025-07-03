// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemBase.h"
#include "InventoryWidget.generated.h"

class AItemBase;
class UTileView;
/**
 * 
 */
UCLASS()
class PROJECTC_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void UpdateItemList(const TArray<FRPGItemData> ItemList);

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTileView> TileView_Items;
};
