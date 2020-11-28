// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Chase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy.h"
#include "EnemyAIController.h"


UBTTask_Chase::UBTTask_Chase() : Super()
{
	NodeName = TEXT("Chase");
}

EBTNodeResult::Type UBTTask_Chase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type NodeResult = Super::ExecuteTask(OwnerComp, NodeMemory);

	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (Enemy == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemy Cast failure"));
		NodeResult = EBTNodeResult::Failed;
	}

	//if (Enemy->EnemyMovementStatus != EEnemyMovementStatus::EMS_Attack) //공격중이 아닐때 chase로 
	{
		Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Chase);
		Enemy->GetCharacterMovement()->MaxWalkSpeed = 550.f;
	}

	return NodeResult;
}