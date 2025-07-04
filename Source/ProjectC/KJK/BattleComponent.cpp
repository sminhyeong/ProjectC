// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "CharacterStateComponent.h"



// Sets default values for this component's properties
UBattleComponent::UBattleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBattleComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UBattleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBattleComponent::Attack(FSkillClass Skill)
{
	AActor* Owner = GetOwner();
	if (!Owner || bIsAttacking || bIsDead) return;

	// �ڸ����� (������ ����)
	if (APawn* PawnOwner = Cast<APawn>(Owner))
	{
		if (AController* Controller = PawnOwner->GetController())
		{
			Controller->StopMovement();
		}
	}

	// ���� ����
	bIsAttacking = true;

	// �ִϸ��̼� ���
	if (UAnimInstance* AnimInstance = Owner->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance())
	{
		if (AttackMontage)
		{
			AnimInstance->Montage_Play(AttackMontage);
			// ���� �ð� �� ���� ����
			GetWorld()->GetTimerManager().SetTimer(
				AttackEndTimerHandle,
				this,
				&UBattleComponent::EndAttack,
				0.8f, // ��Ÿ�� ��ü �ð�
				false
			);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("%s Used / Mana %.1f Used / DMG is %.1f"),
		*Skill.SkillName,
		Skill.SkillCostMana,
		Skill.DMG);

}
void UBattleComponent::EndAttack()
{
	bIsAttacking = false;
}


void UBattleComponent::TryHitTarget(FSkillClass Skill)
{
	//ĳ�����ؼ� �����;��� �͵�
	
	//

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector Start = Owner->GetActorLocation();
	FVector Forward = Owner->GetActorForwardVector();
	FVector End = Start + Forward * 150.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner); // �ڱ� �ڽ��� ����

	FHitResult Hit;
	bool bHit =  UKismetSystemLibrary::SphereTraceSingle(
		this,
		Start,
		End,
		50.f,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		{ Owner },
		EDrawDebugTrace::ForDuration,
		Hit,
		true
	);

	if (bHit && Hit.GetActor())
	{
		UE_LOG(LogTemp, Log, TEXT("Hit %s"), *Hit.GetActor()->GetName());

		// ������ ���̺��� ������ ����
		


		// �ǰ� ����Ʈ, ���� �� ó��
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Skill.HitEffect, Hit.ImpactPoint);
	}
}

void UBattleComponent::OnHit(float CurHP, AActor* Instigate)
{
}

void UBattleComponent::Die(float CurHP)
{
}

