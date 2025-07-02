// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "CharacterStateComponent.h"
#include "BattleComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTC_API UBattleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBattleComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//공격
	UFUNCTION(BlueprintCallable)
	void Attack();

	//공격판정-데미지 강할시 화면 흔들림 조건,FX
	UFUNCTION(BlueprintCallable)
	void TryHitTarget();

	//데미지 여부에따라  화면 흔들림 강도 조건, FX, 유발
	UFUNCTION(BlueprintCallable)
	void OnHit(float CurHP ,AActor* Instigate);

	//체력 0시 발생 - 스폰 등
	UFUNCTION(BlueprintCallable)
	void Die(float CurHP);
};
