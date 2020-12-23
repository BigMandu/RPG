// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	if (Owner == nullptr)
	{
		Owner = TryGetPawnOwner();
		if (Owner)
		{
			Enemy = Cast<AEnemy>(Owner);
		}
	}
}


void UEnemyAnimInstance::UpdateAnimation()
{
	if (Owner == nullptr)
	{
		Owner = TryGetPawnOwner();
		if (Owner)
		{
			Enemy = Cast<AEnemy>(Owner);
		}
	}
	

	if (Owner)
	{
		FVector Speed = Owner->GetVelocity();
		FVector LiteralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LiteralSpeed.Size();
	}
}


void UEnemyAnimInstance::AnimNotify_RangeAttack()
{
	//UE_LOG(LogTemp, Warning, TEXT("EnemyAnimInstance::Range Attack Notify Received!!"));
	if (Enemy->bHasRangeAttack)
	{
		RangeAttack.Broadcast();
	}
	
}


void UEnemyAnimInstance::AnimNotify_AttackEnd()
{
	//UE_LOG(LogTemp, Warning, TEXT("EnemyAnimInstance::AttackEnd Notify Received!!"));
	AttackEnd.Broadcast();
	
}

void UEnemyAnimInstance::AnimNotify_ActivateCollision() //Enemy에서 사용
{
	ActivateCollision.Broadcast();
}
void UEnemyAnimInstance::AnimNotify_DeactivateCollision() //Enemy에서 사용
{
	DeactivateCollision.Broadcast();
}