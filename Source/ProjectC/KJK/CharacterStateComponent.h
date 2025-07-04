// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
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

USTRUCT(BlueprintType)
struct FSkillClass
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DMG;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillCostMana;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)

	UParticleSystem* HitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* SkillMontage;
};



USTRUCT(BlueprintType)
struct FStatMax
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	FString CharacterName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Max")
	float MaxHP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Max")
	float MaxMP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Max")
	float MaxShield;
};

USTRUCT(BlueprintType)
struct FCharacterState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurHP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurMP;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurShield;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	FStatMax MaxStats;
};

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
	float AddDamage(FSkillClass Skill,float& OutHP, float& OutShield);


	UFUNCTION(BlueprintCallable)

	float AddHeal(float Amount, FSkillClass DamageType, AActor* Instigate);

	UFUNCTION(BlueprintCallable)

	float AddMP(float Amount, FSkillClass DamageType, AActor* Instigate);


	UFUNCTION(BlueprintCallable)

	float UseMP(float Amount, FSkillClass DamageType, FCharacterState CharData);
	
	UFUNCTION(BlueprintCallable)

	float AddShield(float Damage, FSkillClass DamageType, AActor* Instigate);

	//캐릭터 스테이트
	UFUNCTION(BlueprintCallable)

	void InitCharacterData(FCharacterState CharData);

	UFUNCTION(BlueprintCallable)

	void SetAdditionalState(FCharacterState AddState);
		
	//변수
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurHP;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float MaxHP;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurShield;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float MaxShield;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurMP;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float MaxMP;

	// 들어오는 데이터가 구조체로 저장되는 변수들
	FCharacterState BaseState;
	FCharacterState AdditionalState;
	FSkillClass Skill;



};
