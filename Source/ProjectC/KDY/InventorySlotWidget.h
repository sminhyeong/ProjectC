// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "InventorySlotWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class PROJECTC_API UInventorySlotWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton>	Button_Item;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Item;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ItemNumber;

public:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> ItemSlotImage;

};
