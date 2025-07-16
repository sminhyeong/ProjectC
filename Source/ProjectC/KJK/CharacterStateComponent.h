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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldChanged, float, CurrentShield, float, MaxShield);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMPChanged, float, CurrentMP, float, MaxMP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDeath, AActor*, DeadCharacter);
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
	
	// 이 함수 추가!
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


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

	// Dispatcher 변수들
	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnShieldChanged OnShieldChanged;

	UPROPERTY(BlueprintAssignable)
	FOnMPChanged OnMPChanged;

	UPROPERTY(BlueprintAssignable)
	FOnCharacterDeath OnCharacterDeath;


	//rep함수
	UFUNCTION(BlueprintCallable)
	virtual void OnRep_CurHP();

	UFUNCTION(BlueprintCallable)
	virtual void OnRep_CurMP();

	UFUNCTION(BlueprintCallable)
	virtual void OnRep_CurShield();
	UFUNCTION(BlueprintCallable)
	virtual void OnRep_MaxHP();

	UFUNCTION(BlueprintCallable)
	virtual void OnRep_MaxMP();

	UFUNCTION(BlueprintCallable)
	virtual void OnRep_MaxShield();

private:
	// Dispatcher 호출을 위한 헬퍼 함수들
	void BroadcastHealthChanged();
	void BroadcastShieldChanged();
	void BroadcastMPChanged();
	void BroadcastCharacterDeath();
};
