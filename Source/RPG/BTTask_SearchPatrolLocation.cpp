// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SearchPatrolLocation.h"
#include "EnemyAIController.h"
#include "Enemy.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_SearchPatrolLocation::UBTTask_SearchPatrolLocation()
{
	NodeName = TEXT("RandomPatrol");
}

EBTNodeResult::Type UBTTask_SearchPatrolLocation::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	AEnemyAIController* AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner()); //AIController�� ������.
	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetCharacter()); //Enemy�� ������.

	//auto ControlEnemy = OwnerComp.GetAIOwner()->GetPawn();

	
	if (BBComp && AICon && Enemy)
	{
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(Enemy->GetWorld());
		if (NavSys)
		{
			Enemy->GetCharacterMovement()->MaxWalkSpeed = 300.f;
			FVector OriginLo = BBComp->GetValueAsVector(AICon->OriginPosKey); //Enemy�� ���� ������ġ�� �������� ����.
			FVector PatrolLo;
			float PatrolArea = 300.f; //���������� ������ ���� ����.

			//������ġ ~ Patrol Area ������ ���� ��ġ�� ����.
			PatrolLo = NavSys->GetRandomReachablePointInRadius(Enemy->GetWorld(), OriginLo, PatrolArea);
			
			BBComp->SetValueAsVector(AICon->PatrolPosKey,PatrolLo); //�� ���� ��ġ�� ����.
			//������
			{
				//UE_LOG(LogTemp, Warning, TEXT("Start From : %s, Move To : %s"), *OriginLo.ToString(), *PatrolLo.ToString());
			}
			return EBTNodeResult::Succeeded;	
		}
		
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Failed;	
}
