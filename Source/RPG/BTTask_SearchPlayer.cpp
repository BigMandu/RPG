// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SearchPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAIController.h"
#include "MainCharacter.h"
#include "Enemy.h"
UBTTask_SearchPlayer::UBTTask_SearchPlayer()
{
	NodeName = TEXT("SearchPlayer");
}


EBTNodeResult::Type UBTTask_SearchPlayer::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	AEnemyAIController* AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	
	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetCharacter());
	FVector EnemyLo = FVector::ZeroVector;
	
	//�� Task�� 3�� �ݺ��� ������. ������ �߰���ġ -> �˻���ġ ->������ �߰���ġ -> �˻���ġ �̷������� �Դٰ��� �Ҳ���.	
	if (BBComp && AICon && Enemy)
	{
		Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Search);

		EnemyLo = (Enemy->GetActorLocation()); //Enemy (AI)�� ��ġ�� �޾ƿ�.
		Enemy->GetCharacterMovement()->MaxWalkSpeed = FMath::RandRange(450.f, 500.f); //���� �ӵ��� 450�� 500���� ������ ���Ѵ�.

		FVector CurrentPlayerLo = (Cast<AMainCharacter>(BBComp->GetValueAsObject(AICon->TargetKey)))->GetActorLocation(); //�÷��̾��� ������ġ
		FVector LastTargetLo = BBComp->GetValueAsVector(AICon->LastPlayerLocationKey); //������ �÷��̾� ��ġ��
		FRotator LastTargetRo = BBComp->GetValueAsRotator(AICon->LastPlayerRotationKey); //ȸ������ �����´�.
		FVector RotationVector = LastTargetRo.Vector(); //������ �÷��̾��� ���⺤�͸� ����. (Yaw��)

		//AICon->MoveToLocation(LastTargetLo); //�켱 ������ ��ġ�� �̵��Ѵ�. Task�� ó���ߴ�.
		
		FVector LocationDist = FVector(FVector::Dist(CurrentPlayerLo, LastTargetLo)); //������ ������ġ�� �÷��̾��� ��ġ�� �Ÿ��� ���ϰ�
		float Rand = FMath::RandRange(LocationDist.Size() / 3, LocationDist.Size()); //�� �Ÿ����� ���� �Ÿ��� ����.
		
		
		FVector PreSearch = FVector(RotationVector.X * Rand, RotationVector.Y * Rand, RotationVector.Z); //���⺤�Ϳ� Rand�� ���ؼ� �� ��ġ�� ����.
		FVector SearchLocation = EnemyLo + PreSearch; //������ġ�� ���� ��ġ�� ���ؼ� ������ġ�� ����.
		
		AICon->MoveToLocation(SearchLocation); //�˻��� ��ġ�� �̵�.

		//������
		{
			UE_LOG(LogTemp, Warning, TEXT("--------------------------------------"));
			UE_LOG(LogTemp, Warning, TEXT("CurPlayerLo : %s, LastPlayerLo : %s"), *CurrentPlayerLo.ToString(), *LastTargetLo.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Vector dist : %s  / Rand value : %f"), *LocationDist.ToString(), Rand);
			UE_LOG(LogTemp, Warning, TEXT("Last Player Rotation : %s, UnitVector : %s"), *LastTargetRo.ToString(), *RotationVector.ToString());

			UE_LOG(LogTemp, Warning, TEXT("Pre Search Location(Local) : %s"), *PreSearch.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Final Search Location(World) : %s"), *SearchLocation.ToString());
		}
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
