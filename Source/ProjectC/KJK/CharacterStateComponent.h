// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "CharacterStateComponent.generated.h"
/* <summary>
 1. ����ü ->���߿� ������ ���̺�� ��ü �� �͵�
 
 2. CPP�Լ�
		1. ���������� : �ǵ� ���� ����
		2. ��
		3. MP ȸ��
		4. MP ���
		5. �ǵ� ȸ��
		6. ĳ���� �ʱ�ȭ
		7. ĳ���� ����
 
*/
USTRUCT(BlueprintType)
struct FDamageClass
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillCostMana;
};

USTRUCT(BlueprintType)
struct FCharacterState
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurHP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHP;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurMP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxShield;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackPower;
};

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

	//������
	UFUNCTION(BlueprintCallable)
	void AddDamage(float Damage, FDamageClass DamageType, AActor* Instigate);


	UFUNCTION(BlueprintCallable)

	void AddHeal(float Amount, FDamageClass DamageType, AActor* Instigate);

	UFUNCTION(BlueprintCallable)

	void AddMP(float Amount, FDamageClass DamageType, AActor* Instigate);


	UFUNCTION(BlueprintCallable)

	void UseMP(float Amount, FDamageClass DamageType, FCharacterState CharData);
	
	UFUNCTION(BlueprintCallable)

	void AddShield(float Damage, FDamageClass DamageType, AActor* Instigate);

	//ĳ���� ������Ʈ
	UFUNCTION(BlueprintCallable)

	void InitCharacterData(FCharacterState CharData);

	UFUNCTION(BlueprintCallable)

	void SetAdditionalState(FCharacterState AddState);
		
	//����
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

	// ������ �����Ͱ� ����ü�� ����Ǵ� ������
	FCharacterState BaseState;
	FCharacterState AdditionalState;
	FDamageClass Skill;
};
