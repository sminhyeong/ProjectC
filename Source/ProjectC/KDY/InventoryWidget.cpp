// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "ItemStruct.h"
#include "Components/TileView.h"
#include "InventorySlotWidget.h"
#include "Components/Image.h"

void UInventoryWidget::UpdateItemList(const TArray<FRPGItemData> ItemList)
{
	TileView_Items->ClearListItems();

	for (auto item : ItemList)
	{
		UInventorySlotWidget* NewItemSlot = CreateWidget<UInventorySlotWidget>(this, InventorySlotWidgetClass);
		if (NewItemSlot)
		{
			NewItemSlot->Image_Item->SetBrushFromTexture(item.ItemInfo.Texture);
			// UE_LOG(LogTemp, Warning, TEXT("ItemTexture : %s"), *item.ItemInfo.Texture.GetName());
			TileView_Items->AddItem(NewItemSlot);
		}
	}
}
