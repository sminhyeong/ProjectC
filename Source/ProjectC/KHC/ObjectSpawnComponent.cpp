// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectSpawnComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UObjectSpawnComponent::UObjectSpawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UObjectSpawnComponent::SpawnObjectAt(TSubclassOf<AActor> ActorClass)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	else
	{
		FTransform SpawnTransform = GetOwner()->GetTransform();
		GetWorld()->SpawnActor<AActor>(ActorClass,SpawnTransform);
	}
}


// Called when the game starts
void UObjectSpawnComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (bAutoSpawn && IsValid(SpawnClass) && !HasSpawned)
	{
		SpawnObjectAt(SpawnClass);
		HasSpawned = true;
	}
	
}


// Called every frame
void UObjectSpawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...   
}

