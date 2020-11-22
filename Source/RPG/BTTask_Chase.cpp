// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Chase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy.h"
#include "EnemyAIController.h"
#include "MainCharacter.h"


UBTTask_Chase::UBTTask_Chase()
{
	NodeName = TEXT("Chase");
}

EBTNodeResult::Type UBTTask_Chase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	AEnemyAIController* AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetOwner());
	AMainCharacter* MainChar = Cast<AMainCharacter>(BBComp->GetValueAsObject(AICon->TargetKey));

	//AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetCharacter());

	if (Enemy && BBComp && AICon && MainChar)
	{
		UE_LOG(LogTemp, Warning, TEXT("-----Chase Task-----"));

		FAIMoveRequest MoveRequest;
		FNavPathSharedPtr PathPtr;

		MoveRequest.SetGoalActor(MainChar);
		MoveRequest.SetAcceptanceRadius(10.f);

		Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Chase);
		Enemy->GetCharacterMovement()->MaxWalkSpeed = 550.f;
		AICon->MoveTo(MoveRequest, &PathPtr);

		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}