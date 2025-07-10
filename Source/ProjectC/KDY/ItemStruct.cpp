// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemStruct.h"

EItemCategory ItemStruct::ConvertTypeToCategory(int32 ItemType)
{
	switch (ItemType)
	{
		case 0: 
			return EItemCategory::WEAPON;
		case 1:
			return EItemCategory::ARMOR;
		case 2:
			return EItemCategory::CONSUME;
		default:
			return EItemCategory::NONE;
	}
}
