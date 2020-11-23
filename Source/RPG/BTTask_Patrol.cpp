// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Patrol.h"
#include "Enemy.h"
#include "EnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_Patrol::UBTTask_Patrol() : Super()
{
	NodeName = TEXT("Patrol");
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type NodeResult =  Super::ExecuteTask(OwnerComp, NodeMemory);
	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (Enemy == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemy Cast failure"));
		NodeResult = EBTNodeResult::Failed;
	}

	Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Patrol);
	Enemy->GetCharacterMovement()->MaxWalkSpeed = 300.f;
	
	return NodeResult;
}
