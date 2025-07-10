// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"
#include "ItemStruct.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UInventorySlotWidget::SetSlotData(FItemAllInfo SlotItem)
{
	ItemData = SlotItem;
	Image_Item->SetBrushFromTexture(SlotItem.ItemArtInfo.Texture);
	Text_ItemNumber->SetText(FText::AsNumber(SlotItem.AccountItemInfo.Quantity));
}
