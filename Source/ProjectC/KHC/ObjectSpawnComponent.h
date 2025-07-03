// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "ObjectSpawnComponent.generated.h"

UENUM(BlueprintType)
enum class ESpawnObjectType : uint8
{
	Monster		UMETA(DisplayName = "Monster"),
	Chest		UMETA(DisplayName = "Chest"),
	ScareCrow	UMETA(DisplayName = "ScareCrow"),
	None		UMETA(DisplayName = "None")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTC_API UObjectSpawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UObjectSpawnComponent();
	UFUNCTION(BlueprintCallable)
	void SpawnObjectAt();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



	UPROPERTY(EditAnywhere, Category = "Spawn")
	bool bAutoSpawn = false;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AActor> SpawnClass;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Spawn")
	ESpawnObjectType SpawnObjectType = ESpawnObjectType::None;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TMap<ESpawnObjectType, TSubclassOf<AActor>> SpawnClassMap;

	TSubclassOf<AActor> GetSpawnClassFromType() const;

	bool HasSpawned = false;
};
