// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SearchPatrolLocation.h"
#include "EnemyAIController.h"
#include "Enemy.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UBTTask_SearchPatrolLocation::UBTTask_SearchPatrolLocation()
{
	NodeName = TEXT("GetPatrolLocation");
}

EBTNodeResult::Type UBTTask_SearchPatrolLocation::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	EBTNodeResult::Type Result = EBTNodeResult::Failed;

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	AEnemyAIController* AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner()); //AIController�� ������.
	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetCharacter()); //Enemy�� ������.

	//auto ControlEnemy = OwnerComp.GetAIOwner()->GetPawn();

	
	if (BBComp && AICon && Enemy)
	{
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(Enemy->GetWorld());
		if (NavSys)
		{
			Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Patrol);
			Enemy->GetCharacterMovement()->MaxWalkSpeed = 300.f;
			FVector OriginLo = BBComp->GetValueAsVector(AICon->OriginPosKey); //Enemy�� ���� ������ġ�� �������� ����.
			FVector PatrolLo;

			//������ġ ~ Patrol Area ������ ���� ��ġ�� ���� ��ġ�� ����.
			PatrolLo = NavSys->GetRandomReachablePointInRadius(Enemy->GetWorld(), OriginLo, Enemy->PatrolArea);
			
			BBComp->SetValueAsVector(AICon->PatrolPosKey,PatrolLo); //���� ��ġ�� Blackboard Key�� �Ѱ���.
			//������
			/*{
				UKismetSystemLibrary::DrawDebugSphere(this, PatrolLo, 50.f, 12, FLinearColor::Green, 4.f, 2.f);
				UE_LOG(LogTemp, Warning, TEXT("Start From : %s, CurLocation : %s, Move To : %s"), *OriginLo.ToString(), *(Enemy->GetActorLocation().ToString()), *PatrolLo.ToString());
			}*/

			
			Result = EBTNodeResult::Succeeded;
		}
	}
	return Result;
}
