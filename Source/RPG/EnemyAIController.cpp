// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "MainCharacter.h"
#include "MainPlayerController.h"
#include "Enemy.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Components/WidgetComponent.h"


AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Enemy = nullptr;

	//////////////////Perception //////////////////
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SenseSightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
	PerceptionComponent->ConfigureSense(*SenseSightConfig);
	SightRadius = 1000.f;
	LoseSightradius = 1500.f;
	VisionAngleDegrees = 80.f;
	MaxAge = 20.f;

	SenseSightConfig->SightRadius = SightRadius;
	SenseSightConfig->LoseSightRadius = LoseSightradius;
	SenseSightConfig->PeripheralVisionAngleDegrees = VisionAngleDegrees; //AI의 시야각 설정
	SenseSightConfig->SetMaxAge(MaxAge);
	SenseSightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	//////////////////Behavior Tree//////////////////

	BTComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BTComp"));
	BBComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BBComp"));

	BBComp->SetValueAsVector(LastPlayerLocationKey, FVector::ZeroVector); //BehaviorTree에 Invalid라고 나와서 그냥 초기화를 해줬다. 
	
}

void AEnemyAIController::BeginPlay() //레벨이 시작될때 호출됨.
{
	Super::BeginPlay();

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::DetectActor);
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Dash Attack(Dash Attack) 가능여부 판단 -> DashAttack Animation이 있으면 가능.
	if (Enemy->DashAttackCombatMontage)
	{
		if (BBComp->GetValueAsBool(HasDetectedPlayerKey))
		{
			AMainCharacter* MainChar = Cast<AMainCharacter>(BBComp->GetValueAsObject(TargetKey));
			if (MainChar)
			{
				FVector TargetLo = MainChar->GetActorLocation();
				float DistanceToTarget = FVector::Dist(TargetLo, Enemy->GetActorLocation());
				if (DistanceToTarget >= SightRadius - 200.f) //포착 && 거리= (시야범위 - 200.f) 이상이면 dashattack가능.
				{
					UpdateCanDashAttack(true);
				}
				else
				{
					UpdateCanDashAttack(false);
				}
			}
		}
	}
	else
	{
		UpdateCanDashAttack(false);
	}


	//상시로 Enum업데이트하기 위해서.
	UpdateEnumMovementStatus(Enemy->EnemyMovementStatus);
}


void AEnemyAIController::DetectActor(AActor* Actor, FAIStimulus Stimulus)
{
	FVector DetectLo = Stimulus.StimulusLocation; //감지된 위치를 저장.
	float DetectedPlayerLostTime = 7.0f; //Target을 초기화 하는 시간.

	if (Actor)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(Actor); //Main으로 캐스트.
		if (Main)
		{
			if (Main->MovementStatus == EMovementStatus::EMS_Dead) //Target이 죽었을때는 감지되지 않도록 한다.
				return;
			if (Stimulus.WasSuccessfullySensed()) //성공적으로 감지를 했으면
			{
				GetWorldTimerManager().ClearTimer(LostTargetTimer); //Timer초기화.

				//감지했을때 Target Key와 HasDetected를 Update해줌.
				UpdateTargetKey(Main);
				UpdateHasDetectedPlayer(true);

				Enemy->EnemyHPbarComp->SetVisibility(true); //Enemy가 Player를 식별했을때 Enemy HP bar를 보여준다.

				//아래는 디버깅.
				{
					//UKismetSystemLibrary::DrawDebugSphere(this, FVector(DetectLo.X, DetectLo.Y, DetectLo.Z + 70.f), 50.f, 12, FLinearColor::Green, 3.f, 2.f);
					////UE_LOG(LogTemp, Warning, TEXT("AI : Detected!! / Target : %s, Location : %s" ), *(Main->GetName()), *DetectLo.ToString());
					//if (GEngine)
					//{
					//	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Detected Location : %s"), *DetectLo.ToString()));
					//}
				}
			}
			else //감지를 못한경우
			{
				LostTargetDelegate = FTimerDelegate::CreateUObject(this, &AEnemyAIController::TargetLost, Actor); //TimerDelegate를 이용해서 파라미터를 넘겨줌
				GetWorldTimerManager().SetTimer(LostTargetTimer, LostTargetDelegate, DetectedPlayerLostTime, false); //SetTimer로 함수를 호출
				
				BBComp->SetValueAsRotator(LastPlayerRotationKey, Main->GetActorRotation());// Player의 회전값과
				BBComp->SetValueAsVector(LastPlayerLocationKey, DetectLo); //마지막 감지 위치를 Blackboard에 넘겨준다.
				UpdateHasDetectedPlayer(false);
				
				//디버깅
				{
					//UKismetSystemLibrary::DrawDebugSphere(this, FVector(DetectLo.X, DetectLo.Y, DetectLo.Z + 70.f), 50.f, 12, FLinearColor::Red, 3.f, 2.f);
					////UE_LOG(LogTemp, Warning, TEXT("AI : Missing Player // Last Location : %s"), *DetectLo.ToString());
					//if (GEngine)
					//{
					//	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Last Location : %s"), *DetectLo.ToString()));
					//}
				}
			}
		}
	}
}

void AEnemyAIController::TargetLost(AActor* Actor)
{
	AMainCharacter* Main = Cast<AMainCharacter>(Actor);

	if (Main)
	{
		//Actor = nullptr; //Main을 null로 바꿔줌

		UpdateTargetKey(nullptr);
		UpdateHasDetectedPlayer(false);

		
		Enemy->EnemyHPbarComp->SetVisibility(false); //Enemy가 Player를 완전히 놓치면 HPbar를 숨긴다.

		//아래는 디버깅
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetLost() // AI : Target Lost!!"));
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("TargetLost() // Target Lost!!")));
			}
		}
	}
}

/***************************************/
//////////// Behavior Tree //////////////
/***************************************/
void AEnemyAIController::OnPossess(APawn * InPawn)
{
	Super::OnPossess(InPawn);

	Enemy = Cast<AEnemy>(InPawn);
	if (Enemy && Enemy->EnemyBehavior)
	{
		FVector Location;
		Location = Enemy->GetActorLocation();

		BBComp->InitializeBlackboard(*(Enemy->EnemyBehavior->BlackboardAsset)); //Blackboard초기화.
		//BBComp->SetValueAsVector(OriginPosKey, Enemy->GetActorLocation()); //월드에 스폰한 위치를 저장
		BBComp->SetValueAsVector(OriginPosKey, FVector(Location.X, Location.Y, 0.f)); //Z를 0으로 해준다. 그냥 GetActorLocation으로 해버리면 SpawnActor가 위에있을때 Z값을 바로 불러와서 원점을 못간다.



		BTComp->StartTree(*(Enemy->EnemyBehavior)); //제일마지막에 StartTree를 해야함.
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't initialize Blackboard"));
	}

	/*
	if (UseBlackboard(BBAsset, Blackboard)) //blackboard Asset과 Component가 성공적으로 연결된다면 true return.
	{
		if (!RunBehaviorTree(BTAsset)) //BehaviorTree를 실행하지 못하면,
		{
			UE_LOG(LogTemp, Warning, TEXT("behavior Tree를 실행할 수 없음"));
		}
		Blackboard->SetValueAsVector(OriginPosKey, InPawn->GetActorLocation()); //OriginPosKey에 현재 Enemy의 위치값을 받아옴.
	}*/


}


/************    Blackboard Key update function   ***********/

//Target Object update 함수.
void AEnemyAIController::UpdateTargetKey(AActor* Target)
{
	AMainCharacter* MainChar = Cast<AMainCharacter>(Target);
	if (MainChar || Target == nullptr)
	{
		BBComp->SetValueAsObject(TargetKey, Target == nullptr ? Target : MainChar);
	}
}

void AEnemyAIController::UpdateHasDetectedPlayer(bool HasDetectedPlayer)
{
	BBComp->SetValueAsBool(HasDetectedPlayerKey, HasDetectedPlayer);
}

void AEnemyAIController::UpdateCanAttack(bool CanAttack)
{
	BBComp->SetValueAsBool(CanAttackKey, CanAttack);
}

void AEnemyAIController::UpdateCanDashAttack(bool CanDashAttack) //EnemyAI -> Tick에서 사용함.
{
	BBComp->SetValueAsBool(CanDashAttackKey, CanDashAttack);
}

void AEnemyAIController::UpdateEnumMovementStatus(EEnemyMovementStatus MovementStatus)
{
	uint8 Value = (uint8)MovementStatus;
	BBComp->SetValueAsEnum(EnumUpdateKey, Value);
}

void AEnemyAIController::UpdateHasDamage(bool HasDamage)
{
	BBComp->SetValueAsBool(HasDamageUpdateKey, HasDamage);
}
void AEnemyAIController::UpdateHasRangeAttack(bool HasRangeAttack)
{
	BBComp->SetValueAsBool(HasRangeAttackKey, HasRangeAttack);
}