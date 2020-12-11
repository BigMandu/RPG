// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FRangeAttackDelegate);
DECLARE_MULTICAST_DELEGATE(FAttackEndDelegate);
/**
 * 
 */
UCLASS()
class RPG_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable, Category = "AnimationProperties")
	void UpdateAnimation();

	UFUNCTION()
	void AnimNotify_RangeAttack(); //AnimNotify가 실행되고, Multi Delegate에 연결된 함수들을 호출하기 위해서
	UFUNCTION()
	void AnimNotify_AttackEnd();

	FRangeAttackDelegate RangeAttack; //이 함수유형을 AnimNotify_RangeAttack함수가 호출될때 전부 호출시킬거다.
	FAttackEndDelegate AttackEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MovementSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	class APawn* Owner;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	class AEnemy* Enemy;
};
