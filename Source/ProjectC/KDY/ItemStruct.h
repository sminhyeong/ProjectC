// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemStruct.generated.h"

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    NONE UMETA(DisplayName = "없음"),
    WEAPON UMETA(DisplayName = "무기"),
    ARMOR UMETA(DisplayName = "방어구"),
    CONSUME UMETA(DisplayName = "소모품"),
};

USTRUCT(BlueprintType)
struct PROJECTC_API FAccountItemInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 ItemID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    FString ItemName;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 ItemType = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 BasePrice = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 AttackBonus = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 DefenseBonus = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 HPBonus = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    int32 MPBonus = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    FString Description;
};

USTRUCT(BlueprintType)
struct FItemArtInfo : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    TObjectPtr<UStaticMesh> StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    TObjectPtr<UTexture2D> Texture;
    
};

USTRUCT(BlueprintType)
struct FItemAllInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    FAccountItemInfo AccountItemInfo;

    UPROPERTY(BlueprintReadWrite, Category = "Item Info")
    FItemArtInfo ItemArtInfo;
};

class PROJECTC_API ItemStruct
{
public:
    static EItemCategory ConvertTypeToCategory(int32 ItemType);
};