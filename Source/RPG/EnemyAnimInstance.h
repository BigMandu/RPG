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
	void AnimNotify_RangeAttack(); //AnimNotify�� ����ǰ�, Multi Delegate�� ����� �Լ����� ȣ���ϱ� ���ؼ�
	UFUNCTION()
	void AnimNotify_AttackEnd();

	FRangeAttackDelegate RangeAttack; //�� �Լ������� AnimNotify_RangeAttack�Լ��� ȣ��ɶ� ���� ȣ���ų�Ŵ�.
	FAttackEndDelegate AttackEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MovementSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	class APawn* Owner;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	class AEnemy* Enemy;
};
