// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    NONE UMETA(DisplayName = "없음"),
    WEAPON UMETA(DisplayName = "무기"),
    ARMOR UMETA(DisplayName = "방어구"),
    CONSUME UMETA(DisplayName = "소모품"),
};

USTRUCT(BlueprintType)
struct FRPGItemInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    int32 Id;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    EItemCategory Category;

    UPROPERTY(BlueprintReadWrite)
    int32 MeshIndex;

    UPROPERTY(BlueprintReadWrite)
    int32 TextureIndex;

    UPROPERTY(BlueprintReadWrite)
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

UCLASS()
class PROJECTC_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
