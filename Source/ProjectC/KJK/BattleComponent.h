// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "BattleComponent.generated.h"

//Setting in Character,Battle,SkillObject.



//�ӽ÷� ����� ����ü- ��ų


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

	//*���� - �⺻ ���ݵ� ��ų�� ����
	// �ڸ� ����, �ִϸ��̼� ��� - ���� ����(TryHitTarget) ����
	
	UFUNCTION(BlueprintCallable)
	void Attack(FSkillClass Skill);
	//���� ����
	UFUNCTION(BlueprintCallable)
	void EndAttack();

	//*��������
	// ������ ���ҽ� ȭ�� ��鸲 ����,FX
	UFUNCTION(BlueprintCallable)
	void TryHitTarget(FSkillClass Skill);

	//*�޴� ������
	//������ ���ο�����  ȭ�� ��鸲 ���� ����, FX, ����
	UFUNCTION(BlueprintCallable)
	void OnHit(float CurHP ,AActor* Instigate);

	//*���
	// ü�� 0�� �߻� - DestroyActor
	UFUNCTION(BlueprintCallable)
	void Die(float CurHP);


	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")

	bool bIsAttacking = false;
	bool bIsDead=false;

	

	FTimerHandle AttackHitTimerHandle;

	FTimerHandle AttackEndTimerHandle;

	UParticleSystem* EmitterTemplate;
};
