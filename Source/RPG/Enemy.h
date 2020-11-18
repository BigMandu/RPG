// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionTypes.h"
#include "Enemy.generated.h"


UENUM(BlueprintType)
enum class EEnemyMovementStatus : uint8
{
	EMS_Idle	UMETA(DisplayName = "Idle"),
	EMS_Chase	UMETA(DisplayName = "Chase"),
	EMS_Attack	UMETA(DisplayName = "Attack"),
	EMS_Dead	UMETA(DisplayName = "Dead"),

	EMS_MAX		UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class RPG_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	///////////////////////////////
	/**     EnemyMovement    ****/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EEnemyMovementStatus EnemyMovementStatus;

	FORCEINLINE void SetEnemyMovementStatus(EEnemyMovementStatus EnemyStatus) { EnemyMovementStatus = EnemyStatus; }

	//////////////////////////////
	/****      Enemy AI      ****/
	//////////////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AEnemyAIController* AIController; //AI Controller클래스

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class USphereComponent* AgroSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	USphereComponent* CombatSphere;

	UPROPERTY(EditAnywhere, Category = "Behavior")
	class UBehaviorTree* EnemyBehavior;

	/*
	//////////////AI TEST////////////////////
	class UNavigationSystemV1* NavSystem; //TEST목적, RandomLocation을 얻는 함수를 사용하기 위해.
	void MoveToRandomLocation();

	
	FTimerHandle LostTimer;
	FTimerDelegate LostDelegate;
	class UAIPerceptionComponent* PerceptionComponent;
	class UAISenseConfig_Sight* SenseSightConfig;

	UFUNCTION()
	void DetectActor(AActor* Actor, FAIStimulus Stimulus);
	void TargetLost(AActor* Actor);
	*/

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//////////////////////////////
	/****      Enemy AI      ****/
	//////////////////////////////
	//UFUNCTION()
	//void Chase(class AMainCharacter* Target);

	UFUNCTION()
	virtual void AgroSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void AgroSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	virtual void CombatSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void CombatSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
