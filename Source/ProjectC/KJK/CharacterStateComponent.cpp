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
//������ - �ǵ� �߰�
void UCharacterStateComponent::AddDamage(float Damage, FDamageClass DamageType, AActor* Instigate)
{
	if (Damage <= 0.f || CurHP <= 0.f)
	
		return;

		UE_LOG(LogTemp, Log, TEXT("AddDamage: %.1f"), Damage);
		if (CurShield > 0.f)
		{
			float ShieldDamage = FMath::Min(Damage, CurShield);
			CurShield -= ShieldDamage;
			Damage -= ShieldDamage;
		}
		// ���� �������� ü�¿� ����
		if (Damage > 0.f)
		{
			CurHP -= Damage;
			CurHP = FMath::Max(0.f, CurHP);
		}

		UE_LOG(LogTemp, Log, TEXT("HP: %.1f / %.1f | Shield: %.1f / %.1f"), CurHP, MaxHP, CurShield, MaxShield);
	

}

//��
void UCharacterStateComponent::AddHeal(float Amount, FDamageClass DamageType, AActor* Instigate)
{
	if (Amount <= 0.f || CurHP <= 0.f)
		return;

	CurHP += Amount;
	CurHP = FMath::Min(CurHP, MaxHP);

	UE_LOG(LogTemp, Log, TEXT("AddHeal: %.1f -> HP: %.1f / %.1f"), Amount, CurHP, MaxHP);

}

//MP ȸ��
void UCharacterStateComponent::AddMP(float Amount, FDamageClass DamageType, AActor* Instigate)
{
	if (Amount <= 0.f || CurMP <= 0.f)
		return;

	CurMP += Amount;
	CurMP = FMath::Min(CurMP, MaxMP);

	UE_LOG(LogTemp, Log, TEXT("AdddMP: %.1f -> HP: %.1f / %.1f"), Amount, CurMP, MaxMP);
}

//MP ���
void UCharacterStateComponent::UseMP(float Amount, FDamageClass DamageType, FCharacterState CharData)
{
	if (Amount <= 0.f || CurMP <= 0.f)
		return;
	Skill = DamageType;
	CharData.CurMP -= Amount;
	CurMP = FMath::Min(CurMP, MaxMP);

	UE_LOG(LogTemp, Log, TEXT("UseMP: %.1f -> MP: %.1f / %.1f"), Amount, CurMP, MaxMP);
}

//�ǵ� �߰�
void UCharacterStateComponent::AddShield(float Amount, FDamageClass DamageType, AActor* Instigate)
{
	if (Amount <= 0.f)
		return;

	CurShield += Amount;
	CurShield = FMath::Min(CurShield, MaxShield);

	UE_LOG(LogTemp, Log, TEXT("AddShield: %.1f -> Shield: %.1f / %.1f"), Amount, CurShield, MaxShield);

}
//ĳ���� �ʱ�ȭ
void UCharacterStateComponent::InitCharacterData(FCharacterState CharData)
{
	BaseState = CharData;

	MaxHP = CharData.MaxHP;
	CurHP = MaxHP;
	MaxMP = CharData.MaxMP;
	CurMP = MaxMP;
	MaxShield = CharData.MaxShield;
	CurShield = MaxShield;

	UE_LOG(LogTemp, Log, TEXT("InitCharacterData -> HP: %.1f | MP: %.1f | Shield: %.1f"), MaxHP, MaxMP,MaxShield);
}

//ĳ���� ���� ����
void UCharacterStateComponent::SetAdditionalState(FCharacterState AddState)
{
	AdditionalState = AddState;

	// ��: ��� �߰� ������ �ݿ��Ϸ��� ���⼭ ���
	MaxHP = BaseState.MaxHP + AdditionalState.MaxHP;
	MaxMP = BaseState.MaxMP + AdditionalState.MaxMP;
	MaxShield = BaseState.MaxShield + AdditionalState.MaxShield;

	// ���� HP,MP, �ǵ�� ���� �� ����
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

