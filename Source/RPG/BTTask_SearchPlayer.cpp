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
	
	//이 Task는 3번 반복할 예정임. 마지막 발견위치 -> 검색위치 ->마지막 발견위치 -> 검색위치 이런식으로 왔다갔다 할꺼임.	
	if (BBComp && AICon && Enemy)
	{
		Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Search);

		EnemyLo = (Enemy->GetActorLocation()); //Enemy (AI)의 위치를 받아옴.
		Enemy->GetCharacterMovement()->MaxWalkSpeed = FMath::RandRange(450.f, 500.f); //수색 속도를 450과 500사이 값으로 정한다.

		FVector CurrentPlayerLo = (Cast<AMainCharacter>(BBComp->GetValueAsObject(AICon->TargetKey)))->GetActorLocation(); //플레이어의 현재위치
		FVector LastTargetLo = BBComp->GetValueAsVector(AICon->LastPlayerLocationKey); //마지막 플레이어 위치와
		FRotator LastTargetRo = BBComp->GetValueAsRotator(AICon->LastPlayerRotationKey); //회전값을 가져온다.
		FVector RotationVector = LastTargetRo.Vector(); //마지막 플레이어의 방향벡터를 구함. (Yaw값)

		//AICon->MoveToLocation(LastTargetLo); //우선 마지막 위치로 이동한다. Task로 처리했다.
		
		FVector LocationDist = FVector(FVector::Dist(CurrentPlayerLo, LastTargetLo)); //마지막 감지위치와 플레이어의 위치의 거리를 구하고
		float Rand = FMath::RandRange(LocationDist.Size() / 3, LocationDist.Size()); //그 거리내의 랜덤 거리를 구함.
		
		
		FVector PreSearch = FVector(RotationVector.X * Rand, RotationVector.Y * Rand, RotationVector.Z); //방향벡터에 Rand를 곱해서 갈 위치를 구함.
		FVector SearchLocation = EnemyLo + PreSearch; //로컬위치에 현재 위치를 더해서 월드위치를 구함.
		
		AICon->MoveToLocation(SearchLocation); //검색한 위치로 이동.

		//디버깅용
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
