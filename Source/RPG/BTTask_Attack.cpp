// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetOwner());

	if (Enemy)
	{
		Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attack);

		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}