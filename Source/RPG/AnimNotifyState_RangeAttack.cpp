// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_RangeAttack.h"
#include "MainCharacter.h"
#include "Enemy.h"


void UAnimNotifyState_RangeAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	OwnerActor = MeshComp->GetOwner();
	Enemy = Cast<AEnemy>(OwnerActor);
	MainChar = Cast<AMainCharacter>(OwnerActor);
	
	if (MainChar)
	{
		MainChar->AttackRangeDamage();
	}
	else if (Enemy && Enemy->bHasRangeAttack)
	{
		Enemy->AttackRangeDamage();
	}
	
}

void UAnimNotifyState_RangeAttack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (Enemy && Enemy->bHasRangeAttack)
	{
		Enemy->RotateToTarget();
		
		if (Enemy->ReturnHit())
		{
			//UE_LOG(LogTemp, Warning, TEXT("Enemy::Hit success"));
			return;
		}
	}
}

void UAnimNotifyState_RangeAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (Enemy && Enemy->bHasRangeAttack)
	{
		Enemy->AttackEnd();
	}
	
}