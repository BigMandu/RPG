// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Enemy.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"


/**
 * 
 */
UCLASS()
class RPG_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AEnemyAIController();

	class AEnemy* Enemy;

	/***************************************/
	//////////// AI Perception //////////////
	/***************************************/
	FTimerHandle LostTargetTimer;
	FTimerDelegate LostTargetDelegate;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	//class UAIPerceptionComponent* PerceptionComponent; 기 선언되어있음.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAISenseConfig_Sight* SenseSightConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception")
	float SightRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception")
	float LoseSightradius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception")
	float VisionAngleDegrees;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception")
	float MaxAge;

	/***************************************/
	//////////// Behavior Tree //////////////
	/***************************************/
	UPROPERTY()
	class UBehaviorTreeComponent* BTComp;
	UPROPERTY()
	class UBlackboardComponent* BBComp;
	

	/***** Blackboard Key *****/
	const FName OriginPosKey = FName(TEXT("OriginPos"));
	const FName PatrolPosKey = FName(TEXT("PatrolPos"));
	const FName TargetKey = FName("Target");
	const FName HasDetectedPlayerKey = FName("HasDetectedPlayer");
	const FName LastPlayerLocationKey = FName("LastPlayerLocationKey");
	const FName LastPlayerRotationKey = FName("LastPlayerRotationKey");
	const FName CanAttackKey = FName("CanAttack");
	const FName CanDashAttackKey = FName("CanDashAttack");
	const FName EnumUpdateKey = FName("EnumStatusKey");
	const FName HasDamageUpdateKey = FName("HasDamage");
	const FName HasRangeAttackKey = FName("HasRangeAttack");

protected:
		// Called when the game starts or when spawned
		virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;


	/***************************************/
	//////////// AI Perception //////////////
	/***************************************/
	UFUNCTION()
	void DetectActor(AActor* Actor, FAIStimulus Stimulus);

	void TargetLost(AActor* Actor);

	/***************************************/
	//////////// Behavior Tree //////////////
	/***************************************/
	virtual void OnPossess(APawn* InPawn) override; //Pawn에 Controller를 붙여주는 함수.


	////  Blackboard Key update function.  
	void UpdateTargetKey(AActor* Target);
	void UpdateHasDetectedPlayer(bool HasDetectedPlayer);
	void UpdateCanAttack(bool CanAttack);
	void UpdateCanDashAttack(bool CanDashAttack);
	void UpdateEnumMovementStatus(EEnemyMovementStatus MovementStatus);
	void UpdateHasDamage(bool HasDamage);
	void UpdateHasRangeAttack(bool HasRangeAttack);
};
