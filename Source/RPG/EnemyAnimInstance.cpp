// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	if (Owner == nullptr)
	{
		Owner = TryGetPawnOwner();
	}
	if (Owner)
	{
		Enemy = Cast<AEnemy>(Owner);
	}
}


void UEnemyAnimInstance::UpdateAnimation()
{
	if (Owner == nullptr)
	{
		Owner = TryGetPawnOwner();
	}
	if (Owner)
	{
		Enemy = Cast<AEnemy>(Owner);
	}

	if (Owner)
	{
		FVector Speed = Owner->GetVelocity();
		FVector LiteralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LiteralSpeed.Size();
	}
}
