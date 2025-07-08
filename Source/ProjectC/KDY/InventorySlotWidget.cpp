// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"
#include "ItemStruct.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UInventorySlotWidget::SetSlotData(FRPGItemData SlotItem)
{
	ItemData = SlotItem;
	Image_Item->SetBrushFromTexture(SlotItem.ItemInfo.Texture);
	Text_ItemNumber->SetText(FText::AsNumber(SlotItem.Number));
}
