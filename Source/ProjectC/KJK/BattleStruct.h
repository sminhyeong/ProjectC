// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleStruct.generated.h"
/**
 * 
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
struct FStatMax : public FTableRowBase
{
	GENERATED_BODY()
public:


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Max")
	float MaxHP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Max")
	float MaxMP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Max")
	float MaxShield;
};

USTRUCT(BlueprintType)
struct FCharacterState : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	FString CharacterName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurHP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurMP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	float CurShield;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	FStatMax MaxStats;
};