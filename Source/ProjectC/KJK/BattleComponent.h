// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "BattleComponent.generated.h"

//Setting in Character,Battle,SkillObject.



//임시로 사용할 구조체- 스킬


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

	//*공격 - 기본 공격도 스킬과 동일
	// 자리 고정, 애니메이션 출력 - 공격 판정(TryHitTarget) 연결
	
	UFUNCTION(BlueprintCallable)
	void Attack(FSkillClass Skill);
	//공격 종료
	UFUNCTION(BlueprintCallable)
	void EndAttack();

	//*공격판정
	// 데미지 강할시 화면 흔들림 조건,FX
	UFUNCTION(BlueprintCallable)
	void TryHitTarget(FSkillClass Skill);

	//*받는 데미지
	//데미지 여부에따라  화면 흔들림 강도 조건, FX, 유발
	UFUNCTION(BlueprintCallable)
	void OnHit(float CurHP ,AActor* Instigate);

	//*사망
	// 체력 0시 발생 - DestroyActor
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
