// Fill out your copyright notice in the Description page of Project Settings.

#include "MainChar_AnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"
#include "MainCharacter.h"

void UMainChar_AnimInstance::NativeInitializeAnimation()
{
	if (Owner == nullptr)
	{
		Owner = TryGetPawnOwner();

		if (Owner) //BP�󿡼� TryGetPawnOwer�� �����ʰ� �ٷ� Main���� ������ �� �ְ�.
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
			if (MainChar == nullptr) //MainChar�� Nullptr�̸� Cast����
			{
				MainChar = Cast<AMainCharacter>(Owner);
			}
		}
	}

	if (Owner)
	{
		FVector Speed = Owner->GetVelocity(); //�ӵ��� ���ѵ�
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f); //����ӵ��� ���� ���Ѵ�.

		MovementSpeed = LateralSpeed.Size();
		bIsinAir = Owner->GetMovementComponent()->IsFalling();

	}

}
