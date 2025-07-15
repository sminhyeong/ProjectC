// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "BattleStruct.h"
#include "CharacterStateComponent.generated.h"
/* <summary>
 1. 구조체 ->나중에 데이터 테이블로 대체 될 것들
 
 2. CPP함수
		1. 데미지적용 : 실드 차감 포함
		2. 힐
		3. MP 회복
		4. MP 사용
		5. 실드 회복
		6. 캐릭터 초기화
		7. 캐릭터 셋팅
 
*/


//----------------------------//
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTC_API UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterStateComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//데미지
	UFUNCTION(BlueprintCallable)
	float AddDamage(float DMGAmount,float& OutHP, float& OutShield);


	UFUNCTION(BlueprintCallable)

	float AddHeal(float Amount, float& OutHP);

	UFUNCTION(BlueprintCallable)

	float AddMP(float Amount, float& OutMP);


	UFUNCTION(BlueprintCallable)

	float UseMP(float Amount, float& OutMP);
	
	UFUNCTION(BlueprintCallable)

	float AddShield(float Damage,float& OutShield);

	UFUNCTION(BlueprintCallable,BlueprintPure)
	bool IsDeath();

	//캐릭터 스테이트
	UFUNCTION(BlueprintCallable)

	void InitCharacterData(FCharacterState CharData);

	UFUNCTION(BlueprintCallable)

	void SetAdditionalState(FCharacterState AddState);
		
	//변수
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurHP, EditAnywhere, Category = "State")
	float CurHP;
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_MaxHP, EditAnywhere, Category = "State")
	float MaxHP;
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurShield, EditAnywhere, Category = "State")
	float CurShield;
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_MaxShield, EditAnywhere, Category = "State")
	float MaxShield;
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurMP, EditAnywhere, Category = "State")
	float CurMP;
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_MaxMP, EditAnywhere,  Category = "State")
	float MaxMP;

	// 들어오는 데이터가 구조체로 저장되는 변수들
	FCharacterState BaseState;
	FCharacterState AdditionalState;
	FSkillClass Skill;


	//rep함수
	UFUNCTION(BlueprintCallable)
	void OnRep_CurHP();

	UFUNCTION(BlueprintCallable)
	void OnRep_CurMP();

	UFUNCTION(BlueprintCallable)
	void OnRep_CurShield();
	UFUNCTION(BlueprintCallable)
	void OnRep_MaxHP();

	UFUNCTION(BlueprintCallable)
	void OnRep_MaxMP();

	UFUNCTION(BlueprintCallable)
	void OnRep_MaxShield();
};
