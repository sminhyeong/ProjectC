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
	if (SpawnObjectType != ESpawnObjectType::Chest)
	{
		if (!GetOwner() || !GetOwner()->HasAuthority())
		{
			return;
		}
	}
	TSubclassOf<AActor> SelectedClass = GetSpawnClassFromType();
	if (!SelectedClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("선택된 클래스가 없습니다."));
		return;
	}
	FTransform SpawnTransform = GetOwner()->GetTransform();
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SelectedClass, SpawnTransform);
	if (!SpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnedActor가 nullptr입니다."));
		return;
	}

	if (SpawnObjectType == ESpawnObjectType::Monster)
	{
		static FName PortalSpawnerName(TEXT("DungeonPortalSpawner"));

		FObjectProperty* ObjectProp = FindFProperty<FObjectProperty>(SpawnedActor->GetClass(), PortalSpawnerName);
		if (ObjectProp && ObjectProp->PropertyClass->IsChildOf(AActor::StaticClass()))
		{
			ObjectProp->SetObjectPropertyValue_InContainer(SpawnedActor, PortalSpawnerActor);
		}

		// 리플렉션을 통해 변수 찾기 및 설정
		UClass* SpawnedClass = SpawnedActor->GetClass();

		FProperty* SpawnerProperty = SpawnedClass->FindPropertyByName(FName("Spawner"));
		if (SpawnerProperty)
		{
			FObjectProperty* ObjectProp2 = CastField<FObjectProperty>(SpawnerProperty);
			if (ObjectProp2 && ObjectProp2->PropertyClass->IsChildOf(AActor::StaticClass()))
			{
				ObjectProp2->SetObjectPropertyValue_InContainer(SpawnedActor, GetOwner());
				UE_LOG(LogTemp, Log, TEXT("Spawner 정보를 보스에게 전달 완료"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Spawner 프로퍼티 타입이 올바르지 않습니다."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnedActor에 Spawner 프로퍼티가 없습니다."));
		}
	}
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

	UE_LOG(LogTemp, Warning, TEXT("SpawnClassMap에 해당 타입이 없거나 비어 있습니다!"));
	return nullptr;
}

