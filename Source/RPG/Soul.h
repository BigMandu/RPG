// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Soul.generated.h"

UCLASS()
class RPG_API ASoul : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASoul();

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soul")
	float Min;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soul")
	float Max;

	�̰� ����ؼ� Rand�� �ѵ�, Delay -> Set Simulate Physics�� ����� (Target �� Sphere) �Ƹ� �յ� ���ٴϴ� ��ȿ���� ���ؼ��ε�?
	*/
	
	float Time;
	float AlphaTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soul")
	int32 SoulCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Soul")
	class USphereComponent* Sphere;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soul | Particle")
	class UParticleSystemComponent* SoulParticle;*/ //Editor���� ���� ������Ʈ�� ������.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soul | Sound")
	class USoundCue* IdleSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soul | Sound")
	USoundCue* DestorySound;

	FVector InitialLocation;

	class AMainCharacter* MainPlayer;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool bInterp;
	void ToggleInterp();

	void CollectSoul();

};
