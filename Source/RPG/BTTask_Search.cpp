// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Search.h"
#include "Enemy.h"
#include "EnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_Search::UBTTask_Search() : Super()
{
	NodeName = TEXT("Search LastPoint");
}

EBTNodeResult::Type UBTTask_Search::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type NodeResult = Super::ExecuteTask(OwnerComp, NodeMemory);
	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetCharacter());

	if (Enemy == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemy Cast failure"));
		NodeResult = EBTNodeResult::Failed;
	}

	Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Search);
	Enemy->GetCharacterMovement()->MaxWalkSpeed = 450.f;

	return NodeResult;
}