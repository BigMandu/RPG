// Fill out your copyright notice in the Description page of Project Settings.

#include "MainChar_AnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"
#include "MainCharacter.h"

void UMainChar_AnimInstance::NativeInitializeAnimation()
{
	if (Owner == nullptr)
	{
		Owner = TryGetPawnOwner();

		if (Owner) //BP상에서 TryGetPawnOwer를 하지않고 바로 Main으로 접근할 수 있게.
		{
			MainChar = Cast<AMainCharacter>(Owner);
		}
	}
}

void UMainChar_AnimInstance::UpdateAnimationProperties()
{
	if (Owner == nullptr)
	{
		Owner = TryGetPawnOwner();
		if (Owner)
		{
			if (MainChar == nullptr) //MainChar가 Nullptr이면 Cast해줌
			{
				MainChar = Cast<AMainCharacter>(Owner);
			}
		}
	}

	if (Owner)
	{
		FVector Speed = Owner->GetVelocity(); //속도를 구한뒤
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f); //수평속도를 따로 구한다.

		MovementSpeed = LateralSpeed.Size();
		bIsinAir = Owner->GetMovementComponent()->IsFalling();

	}

}
