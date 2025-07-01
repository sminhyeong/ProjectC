// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectSpawnComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTC_API UObjectSpawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UObjectSpawnComponent();
	UFUNCTION(BlueprintCallable)
	void SpawnObjectAt(TSubclassOf<AActor> ActorClass);

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

	bool HasSpawned = false;
};
