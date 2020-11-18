// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SearchPatrolLocation.h"
#include "EnemyAIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SearchPatrolLocation::UBTTask_SearchPatrolLocation()
{
	NodeName = TEXT("SearchPatrolLocation");
}

EBTNodeResult::Type UBTTask_SearchPatrolLocation::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	AEnemyAIController* AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner()); //AIController¸¦ °¡Á®¿È.
	
	auto ControlEnemy = OwnerComp.GetAIOwner()->GetPawn();

	if (ControlEnemy && AICon)
	{
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(ControlEnemy->GetWorld());
		if (NavSys)
		{
			FVector OriginLo = BBComp->GetValueAsVector(AICon->OriginPosKey);
			FVector PatrolLo;
			PatrolLo = NavSys->GetRandomReachablePointInRadius(ControlEnemy->GetWorld(), OriginLo, 1000.f);
			
			BBComp->SetValueAsVector(AICon->PatrolPosKey,PatrolLo);
			//µð¹ö±ë¿ë
			{
				UE_LOG(LogTemp, Warning, TEXT("OriginLo : %s, PatrolLo : %s"), *OriginLo.ToString(), *PatrolLo.ToString());
			}
			return EBTNodeResult::Succeeded;	
		}
		else
		{
			return EBTNodeResult::Failed;
		}
	}
	else
	{
		return EBTNodeResult::Failed;
	}
	
}
