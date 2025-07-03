// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "Components/TileView.h"
#include "InventorySlotWidget.h"
#include "Components/Image.h"
// #include "Styling/SlateBrush.h"

void UInventoryWidget::UpdateItemList(const TArray<FRPGItemData> ItemList)
{
	TileView_Items->ClearListItems();

	for (auto item : ItemList)
	{
		UInventorySlotWidget* NewItemSlot = CreateWidget<UInventorySlotWidget>(this, UInventorySlotWidget::StaticClass());
		if (NewItemSlot)
		{
			// FSlateBrush NewBrush = NewItemSlot->Image_Item->GetBrush();
			// NewBrush.SetResourceObject(item.ItemInfo.TextureIndex);
			// NewItemSlot->Image_Item->SetBrush(NewBrush);
			TileView_Items->AddItem(NewItemSlot);
		}
	}
}
