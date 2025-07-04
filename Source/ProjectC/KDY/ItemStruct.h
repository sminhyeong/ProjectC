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
struct FRPGItemInfo : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UTexture2D> Texture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
};

USTRUCT(BlueprintType)
struct FRPGItemData
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    FRPGItemInfo ItemInfo;

    // 아이템 개수
    UPROPERTY(BlueprintReadWrite)
    int32 Number;
};
