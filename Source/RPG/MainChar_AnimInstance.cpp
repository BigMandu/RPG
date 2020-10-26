// Fill out your copyright notice in the Description page of Project Settings.

#include "MainChar_AnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"

void UMainChar_AnimInstance::NativeInitializeAnimation()
{
	if (Owner == nullptr)
	{
		Owner = TryGetPawnOwner();
	}
}

void UMainChar_AnimInstance::UpdateAnimationProperties()
{
	if (Owner == nullptr)
	{
		Owner = TryGetPawnOwner();
	}

	if (Owner)
	{
		FVector Speed = Owner->GetVelocity(); //속도를 구한뒤
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f); //수평속도를 따로 구한다.

		MovementSpeed = LateralSpeed.Size();

		bIsinAir = Owner->GetMovementComponent()->IsFalling();

	}

}
