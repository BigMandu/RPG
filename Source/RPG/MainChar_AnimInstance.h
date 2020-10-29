// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MainChar_AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API UMainChar_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	virtual void NativeInitializeAnimation() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class APawn* Owner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class AMainCharacter* MainChar; //BP상에서 Get Pawn owner를 하지 않고 Main에 바로 접근하기 위해서

	UFUNCTION(BlueprintCallable, Category = AnimationProperties)
	void UpdateAnimationProperties();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsinAir;

	
};
