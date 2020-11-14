// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
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

	/***************************************/
	//////////// AI Perception //////////////
	/***************************************/
	FTimerHandle LostTargetTimer;
	FTimerDelegate LostTargetDelegate;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	//class UAIPerceptionComponent* PerceptionComponent; 기 선언되어있음.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAISenseConfig_Sight* SenseSightConfig;
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	//class UNavigationSystemV1* NavSystem; //TEST목적, RandomLocation을 얻는 함수를 사용하기 위해.
	
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
	class UBehaviorTree* BTAsset;
	UPROPERTY()
	class UBlackboardData* BBAsset;
	



protected:
		// Called when the game starts or when spawned
		virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;


	void MoveToRandomLocation();
	/***************************************/
	//////////// AI Perception //////////////
	/***************************************/
	UFUNCTION()
	void DetectActor(AActor* Actor, FAIStimulus Stimulus);

	void TargetLost(AActor* Actor);

	UFUNCTION()
	void Chase(class AActor* Chaser, class AMainCharacter* Target);

	/***************************************/
	//////////// Behavior Tree //////////////
	/***************************************/
	virtual void OnPossess(APawn* InPawn) override;

};
