
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectSpawnComponent.h"
#include "ObjectSpawner.generated.h"


UCLASS()
class PROJECTC_API AObjectSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectSpawner();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UObjectSpawnComponent* SpawnComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
