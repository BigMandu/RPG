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
		FVector Speed = Owner->GetVelocity(); //�ӵ��� ���ѵ�
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f); //����ӵ��� ���� ���Ѵ�.

		MovementSpeed = LateralSpeed.Size();

		bIsinAir = Owner->GetMovementComponent()->IsFalling();

	}

}
