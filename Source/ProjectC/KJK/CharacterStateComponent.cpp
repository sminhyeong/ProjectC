// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStateComponent.h"

// Sets default values for this component's properties
UCharacterStateComponent::UCharacterStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;



	// ...
}
//데미지 - 실드 추가
float UCharacterStateComponent::AddDamage(FSkillClass InSkill, float& OutHP, float& OutShield)
{


	if (InSkill.DMG <= 0.f || CurHP <= 0.f)
	

		UE_LOG(LogTemp, Log, TEXT("AddDamage: %.1f"), InSkill.DMG);
		if (CurShield > 0.f)
		{
			float ShieldDamage = FMath::Min(InSkill.DMG, CurShield);
			CurShield -= ShieldDamage;
			InSkill.DMG -= ShieldDamage;
		}
		// 남은 데미지를 체력에 적용
		if (InSkill.DMG > 0.f)
		{
			CurHP -= InSkill.DMG;
			CurHP = FMath::Max(0.f, CurHP);
		}
	

		UE_LOG(LogTemp, Log, TEXT("HP: %.1f / %.1f | Shield: %.1f / %.1f"), CurHP, MaxHP, CurShield, MaxShield);
		
		return InSkill.DMG;
		
	
}

//힐
float UCharacterStateComponent::AddHeal(float Amount, FSkillClass DamageType, AActor* Instigate)
{
	if (Amount <= 0.f || CurHP <= 0.f)

	CurHP += Amount;
	CurHP = FMath::Min(CurHP, MaxHP);

	UE_LOG(LogTemp, Log, TEXT("AddHeal: %.1f -> HP: %.1f / %.1f"), Amount, CurHP, MaxHP);
		return CurHP;

}

//MP 회복
float UCharacterStateComponent::AddMP(float Amount, FSkillClass DamageType, AActor* Instigate)
{
	if (Amount <= 0.f || CurMP <= 0.f)

	CurMP += Amount;
	CurMP = FMath::Min(CurMP, MaxMP);

	UE_LOG(LogTemp, Log, TEXT("AdddMP: %.1f -> HP: %.1f / %.1f"), Amount, CurMP, MaxMP);
		return CurMP;
}

//MP 사용
float UCharacterStateComponent::UseMP(float Amount, FSkillClass DamageType, FCharacterState CharData)
{
	if (Amount <= 0.f || CurMP <= 0.f)
	Skill = DamageType;
	CharData.CurMP -= Amount;
	CurMP = FMath::Min(CurMP, MaxMP);

	UE_LOG(LogTemp, Log, TEXT("UseMP: %.1f -> MP: %.1f / %.1f"), Amount, CurMP, MaxMP);
		return CurMP;
}

//실드 추가
float UCharacterStateComponent::AddShield(float Amount, FSkillClass DamageType, AActor* Instigate)
{
	if (Amount <= 0.f)

	CurShield += Amount;
	CurShield = FMath::Min(CurShield, MaxShield);

	UE_LOG(LogTemp, Log, TEXT("AddShield: %.1f -> Shield: %.1f / %.1f"), Amount, CurShield, MaxShield);
		return CurShield;
}
//SetUpCharacter
void UCharacterStateComponent::InitCharacterData(FCharacterState CharData)
{
	BaseState = CharData;

	MaxHP = CharData.MaxStats.MaxHP;	
	CurHP = MaxHP;
	MaxMP = CharData.MaxStats.MaxMP;
	CurMP = MaxMP;
	MaxShield = CharData.MaxStats.MaxShield;
	CurShield = MaxShield;

	UE_LOG(LogTemp, Log, TEXT("InitCharacterData -> HP: %.1f | MP: %.1f | Shield: %.1f"), CurHP, CurMP, CurShield);
}

//캐릭터 상태 셋팅
void UCharacterStateComponent::SetAdditionalState(FCharacterState AddState)
{
	AdditionalState = AddState;

	// 예: 장비 추가 스탯을 반영하려면 여기서 계산
	MaxHP = BaseState.MaxStats.MaxHP + AdditionalState.MaxStats.MaxHP;
	MaxMP = BaseState.MaxStats.MaxMP + AdditionalState.MaxStats.MaxMP;
	MaxShield = BaseState.MaxStats.MaxShield + AdditionalState.MaxStats.MaxShield;

	// 현재 HP,MP, 실드는 기존 값 유지
	UE_LOG(LogTemp, Log, TEXT("SetAdditionalState -> Total MaxHP: %.1f | MP: %.1f | MaxShield: %.1f"), MaxHP, MaxMP, MaxShield);
}


// Called when the game starts
void UCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCharacterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

