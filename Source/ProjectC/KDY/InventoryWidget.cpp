// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "ItemStruct.h"
#include "Components/TileView.h"
#include "Components/TextBlock.h"
#include "InventorySlotWidget.h"
#include "Components/Image.h"

void UInventoryWidget::UpdateItemList(const TArray<FItemAllInfo> ItemList)
{
	TileView_Items->ClearListItems();

	if (ItemList.Num() > 0)
	{
		NowWatchCategory = ItemStruct::ConvertTypeToCategory(ItemList[0].AccountItemInfo.ItemType);

		for (auto item : ItemList)
		{
			UInventorySlotWidget* NewItemSlot = CreateWidget<UInventorySlotWidget>(this, InventorySlotWidgetClass);
			if (NewItemSlot)
			{
				NewItemSlot->SetSlotData(item);
				TileView_Items->AddItem(NewItemSlot);
			}
		}
		
	}
}
