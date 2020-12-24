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


	Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Chase);
	Enemy->GetCharacterMovement()->MaxWalkSpeed = 550.f;

	//UE_LOG(LogTemp, Warning, TEXT("Enemy name is : %s"),*(Enemy->GetFName().ToString()));  result is  'MinionLaneCore_dusk_BP_3'

	if (Enemy->GetFName().ToString().Contains(TEXT("Lane"), ESearchCase::IgnoreCase, ESearchDir::FromStart)) //Enemy별로 Chase 속도를 다르게 하기 위함.
	{
		if (Enemy->GetFName().ToString().Contains(TEXT("Core"), ESearchCase::IgnoreCase, ESearchDir::FromStart))
		{
			Enemy->GetCharacterMovement()->MaxWalkSpeed = 950.f;
		}
		else
		{
			Enemy->GetCharacterMovement()->MaxWalkSpeed = 720.f;
		}

	}

	return NodeResult;
}