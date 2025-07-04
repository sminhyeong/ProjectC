// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KDY/ItemStruct.h"
#include "RewardActorComponent.generated.h"

UENUM(BlueprintType)
enum class ERewardGiveType : uint8
{
	Boss		UMETA(DisplayName = "Boss"),
	Chest		UMETA(DisplayName = "Chest"),
	None		UMETA(DisplayName = "None")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTC_API URewardActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URewardActorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TArray<FItemStruct> RewardItems;

	UFUNCTION(BlueprintCallable)
	void RewardToPlayer(AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable)
	void RewardWidget(const TArray<FItemStruct>& ItemToShow, AActor* PlayerActor);
		
};
