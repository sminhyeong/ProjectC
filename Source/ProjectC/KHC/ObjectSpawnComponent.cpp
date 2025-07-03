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

void UObjectSpawnComponent::SpawnObjectAt()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	TSubclassOf<AActor> SelectedClass = GetSpawnClassFromType();
	if (!SelectedClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("���õ� Ŭ������ �����ϴ�."));
		return;
	}
	FTransform SpawnTransform = GetOwner()->GetTransform();
	GetWorld()->SpawnActor<AActor>(SelectedClass,SpawnTransform);
}


// Called when the game starts
void UObjectSpawnComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (bAutoSpawn && !HasSpawned)
	{	
		SpawnObjectAt();
		HasSpawned = true;
	}
}


// Called every frame
void UObjectSpawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...   
}

TSubclassOf<AActor> UObjectSpawnComponent::GetSpawnClassFromType() const
{
	const TSubclassOf<AActor>* FoundClass = SpawnClassMap.Find(SpawnObjectType);

	if (FoundClass && *FoundClass)
	{
		return *FoundClass;
	}

	UE_LOG(LogTemp, Warning, TEXT("SpawnClassMap�� �ش� Ÿ���� ���ų� ��� �ֽ��ϴ�!"));
	return nullptr;
}

