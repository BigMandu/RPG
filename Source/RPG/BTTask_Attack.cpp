// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy.h"
#include "EnemyAIController.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	bNotifyTick = true;	
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetCharacter());
	if (Enemy && Enemy->bAttacking == false)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Patrol); //Attack�� �ƴѰ����� �������ش�.
	}
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type NodeResult = EBTNodeResult::InProgress;
	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetCharacter());
	AEnemyAIController* AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	
	if (Enemy == nullptr || AICon == nullptr)
	{
		NodeResult = EBTNodeResult::Failed;
	}
	//Enemy->RotateToTarget(OwnerComp.GetBlackboardComponent(), AICon); //Target���� ȸ��.
	Enemy->Attack(OwnerComp.GetBlackboardComponent());

	return NodeResult;
}
