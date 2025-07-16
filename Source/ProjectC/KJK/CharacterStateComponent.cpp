// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStateComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCharacterStateComponent::UCharacterStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	SetIsReplicatedByDefault(true);


	// ...
}
//데미지 - 실드 추가
float UCharacterStateComponent::AddDamage(float DMGAmount, float& OutHP, float& OutShield)
{

	if (DMGAmount <= 0.f || CurHP <= 0.f)
	{
		CurHP = 0.f;
		return  0;
	}

	UE_LOG(LogTemp, Log, TEXT("AddDamage: %.1f"), DMGAmount);
	if (CurShield > 0.f)
	{
		float ShieldDamage = FMath::Min(DMGAmount, CurShield);
		CurShield -= ShieldDamage;
		DMGAmount -= ShieldDamage;
	}
	// 남은 데미지를 체력에 적용
	if (DMGAmount > 0.f)
	{
		CurHP -= DMGAmount;
		CurHP = FMath::Max(0.f, CurHP);

	}
	OutShield = CurShield;
	OutHP = CurHP;

	UE_LOG(LogTemp, Log, TEXT("HP: %.1f / %.1f | Shield: %.1f / %.1f"), OutHP, MaxHP, OutShield, MaxShield);

	return DMGAmount;


}

//힐
float UCharacterStateComponent::AddHeal(float Amount, float& OutHP)
{
	if (Amount <= 0.f || CurHP <= 0.f)
	{
		return CurHP;
	}
	CurHP += Amount;
	CurHP = FMath::Min(CurHP, MaxHP);

	OutHP = CurHP;
	UE_LOG(LogTemp, Log, TEXT("AddHeal: %.1f -> HP: %.1f / %.1f"), Amount, CurHP, MaxHP);
	return CurHP;

}

//MP 회복
float UCharacterStateComponent::AddMP(float Amount, float& OutMP)
{
	if (Amount <= 0.f || CurMP >= MaxMP)
	{
		return CurMP;
	}
	CurMP += Amount;
	CurMP = FMath::Min(CurMP, MaxMP);
	OutMP = CurMP;
	UE_LOG(LogTemp, Log, TEXT("AddMP: %.1f -> MP: %.1f / %.1f"), Amount, CurMP, MaxMP);
	return CurMP;
}

//MP 사용
float UCharacterStateComponent::UseMP(float Amount, float& OutMP)
{
	if (Amount <= 0.f || CurMP <= 0.f)
	{
		return CurMP;
	}
	CurMP -= Amount;
	CurMP = FMath::Max(0.f, CurMP);

	OutMP = CurMP;
	UE_LOG(LogTemp, Log, TEXT("UseMP: %.1f -> MP: %.1f / %.1f"), Amount, CurMP, MaxMP);
	return CurMP;
}

//실드 추가
float UCharacterStateComponent::AddShield(float Amount, float& OutShield)
{
	if (Amount <= 0.f || CurShield <= 0.f)
	{
		return CurShield;
	}
	CurShield += Amount;
	CurShield = FMath::Min(CurShield, MaxShield);

	OutShield = CurShield;
	UE_LOG(LogTemp, Log, TEXT("AddShield: %.1f -> Shield: %.1f / %.1f"), Amount, CurShield, MaxShield);
	return CurShield;
}
bool UCharacterStateComponent::IsDeath()
{
	if (CurHP <= 0.f)
	{
		//CurHP = 0.f;
		return true;
	}
	return false;
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

	UE_LOG(LogTemp, Warning, TEXT("InitCharacterData -> HP: %.1f | MP: %.1f | Shield: %.1f"), CurHP, MaxHP, CurShield);
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

void UCharacterStateComponent::OnRep_CurHP()
{
	BroadcastHealthChanged();
}

void UCharacterStateComponent::OnRep_CurMP()
{
	BroadcastMPChanged();
}

void UCharacterStateComponent::OnRep_CurShield()
{
	BroadcastShieldChanged();
}

void UCharacterStateComponent::OnRep_MaxHP()
{
	BroadcastHealthChanged();
}

void UCharacterStateComponent::OnRep_MaxMP()
{
	BroadcastMPChanged();
}

void UCharacterStateComponent::OnRep_MaxShield()
{
	BroadcastShieldChanged();
}

void UCharacterStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterStateComponent, MaxHP);      // 이게 빠져있었음!
	DOREPLIFETIME(UCharacterStateComponent, CurHP);
	DOREPLIFETIME(UCharacterStateComponent, MaxShield);  // 이것도 빠져있었음!
	DOREPLIFETIME(UCharacterStateComponent, CurShield);
	DOREPLIFETIME(UCharacterStateComponent, MaxMP);      // 이것도 빠져있었음!
	DOREPLIFETIME(UCharacterStateComponent, CurMP);
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

void UCharacterStateComponent::BroadcastHealthChanged()
{
	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(CurHP, MaxHP);
	}
}

void UCharacterStateComponent::BroadcastShieldChanged()
{
	if (OnShieldChanged.IsBound())
	{
		OnShieldChanged.Broadcast(CurShield, MaxShield);
	}
}

void UCharacterStateComponent::BroadcastMPChanged()
{
	if (OnMPChanged.IsBound())
	{
		OnMPChanged.Broadcast(CurMP, MaxMP);
	}
}

void UCharacterStateComponent::BroadcastCharacterDeath()
{
	if (OnCharacterDeath.IsBound())
	{
		OnCharacterDeath.Broadcast(GetOwner());
	}
}
