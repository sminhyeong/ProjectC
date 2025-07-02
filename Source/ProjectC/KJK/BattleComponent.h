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

	//����
	UFUNCTION(BlueprintCallable)
	void Attack();

	//��������-������ ���ҽ� ȭ�� ��鸲 ����,FX
	UFUNCTION(BlueprintCallable)
	void TryHitTarget();

	//������ ���ο�����  ȭ�� ��鸲 ���� ����, FX, ����
	UFUNCTION(BlueprintCallable)
	void OnHit(float CurHP ,AActor* Instigate);

	//ü�� 0�� �߻� - ���� ��
	UFUNCTION(BlueprintCallable)
	void Die(float CurHP);
};
