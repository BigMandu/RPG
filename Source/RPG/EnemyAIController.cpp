// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "MainCharacter.h"
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


AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	//////////////////Perception //////////////////
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SenseSightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
	PerceptionComponent->ConfigureSense(*SenseSightConfig);
	SightRadius = 1000.f;
	LoseSightradius = 1500.f;
	VisionAngleDegrees = 60.f;
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
	
	//MoveRandom test, timerhandle
	/*
	FTimerHandle RandomTimerHandle;
	GetWorldTimerManager().SetTimer(RandomTimerHandle, this, &AEnemyAIController::MoveToRandomLocation, 3.0f, true);
	*/
	//NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::DetectActor);
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
			if (Stimulus.WasSuccessfullySensed()) //성공적으로 감지를 했으면
			{
				GetWorldTimerManager().ClearTimer(LostTargetTimer); //Timer초기화.

				//감지했을때 Target Key와 HasDetected를 Update해줌.
				UpdateTargetKey(Actor);
				UpdateHasDetectedPlayer(true);

				//아래는 디버깅.
				{
					UKismetSystemLibrary::DrawDebugSphere(this, FVector(DetectLo.X, DetectLo.Y, DetectLo.Z + 70.f), 50.f, 12, FLinearColor::Green, 3.f, 2.f);
					UE_LOG(LogTemp, Warning, TEXT("AI : Detected!! / Target : %s, Location : %s" ), *(Actor->GetName()), *DetectLo.ToString());
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Detected Location : %s"), *DetectLo.ToString()));
					}
				}
			}
			else //감지를 못한경우
			{
				LostTargetDelegate = FTimerDelegate::CreateUObject(this, &AEnemyAIController::TargetLost, Actor); //TimerDelegate를 이용해서 파라미터를 넘겨줌
				GetWorldTimerManager().SetTimer(LostTargetTimer, LostTargetDelegate, DetectedPlayerLostTime, false); //SetTimer로 함수를 호출
				
				
				BBComp->SetValueAsRotator(LastPlayerRotationKey, Main->GetActorRotation());// Player의 회전값도 넣어준다.
				BBComp->SetValueAsVector(LastPlayerLocationKey, DetectLo); //마지막 감지 위치를 LastPlyaerLocationKey에 넘겨준다.
				UpdateHasDetectedPlayer(false);
				
				//디버깅
				{
					UKismetSystemLibrary::DrawDebugSphere(this, FVector(DetectLo.X, DetectLo.Y, DetectLo.Z + 70.f), 50.f, 12, FLinearColor::Red, 3.f, 2.f);
					UE_LOG(LogTemp, Warning, TEXT("AI : Missing Player // Last Location : %s"), *DetectLo.ToString());
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Last Location : %s"), *DetectLo.ToString()));
					}
				}
			}
		}
	}
}

void AEnemyAIController::TargetLost(AActor* Actor)
{
	Actor = nullptr; //Main을 null로 바꿔줌
	
	UpdateTargetKey(nullptr);
	UpdateHasDetectedPlayer(false);

	//아래는 디버깅
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetLost() // AI : Target Lost!!"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("TargetLost() // Target Lost!!")));
		}
	}
}

/*
void AEnemyAIController::MoveToRandomLocation() //TEST목적
{
	//AEnemy* Enemy = GetWorld()->SpawnActor<AEnemy>(); //Enemy를 계속 SapwnActor로 하기때문에 NULL이 생길때도 있다.
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	if (NavSys)//NavSystem)
	{
		FVector Origin = FVector::ZeroVector;
		FVector RandomLocation = FVector(NavSys->GetRandomReachablePointInRadius(this, Origin, 2500.f));

		//NavSystem->SimpleMoveToLocation(AIController, LinearRandLocation);
		MoveToLocation(RandomLocation);//LinearRandLocation);

		//디버깅
		{
			UKismetSystemLibrary::DrawDebugSphere(this, RandomLocation, 25.f, 6, FLinearColor::Blue, 3.f, 2.f);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Next Location : %s"), *RandomLocation.ToString()));
			}
		}
	}
}
*/

/*
void AEnemyAIController::Chase(AActor* Chaser, AMainCharacter* Target)
{
	AEnemy* Enemy = Cast<AEnemy>(Chaser);

	if (Enemy->AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy Movement : Chase "));

		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(15.f);
		
		FNavPathSharedPtr NavPath;

		MoveTo(MoveRequest, &NavPath);
	}
}
*/

//망한 함수
/*
void AEnemyAIController::SearchAndMove(FVector LastLocation, FVector TargetLocation)
{
	FVector LocationDist = FVector(FVector::Dist(TargetLocation, LastLocation));
	FTimerHandle MoveTimer;
	//float Ratio = 1.0;
	AEnemy* Enemy = Cast<AEnemy>(AEnemyAIController::GetCharacter());
	EPathFollowingRequestResult::Type MoveResult;

	if (Enemy == nullptr) return;

	UE_LOG(LogTemp, Warning, TEXT(" Search Player "));
	for (int32 lo = 0; lo < 3; lo++) //우선 3번 반복
	{
		FVector CurLo = Enemy->GetActorLocation();

		float Rand = FMath::RandRange(LocationDist.Size() / 3, LocationDist.Size() / 1.5);
		FVector SearchLocation = FVector(CurLo.X + Rand, CurLo.Y + Rand, CurLo.Z);

		//디버깅용
		{
			UE_LOG(LogTemp, Warning, TEXT("----------- Loop : %d -----------"), lo);
			//UE_LOG(LogTemp, Warning, TEXT("LastLo : %s, TargetLo : %s"), *LastLocation.ToString(), *TargetLocation.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Current Enemy Location : %s"), *CurLo.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Vector dist : %s, float Rand value : %f"), *LocationDist.ToString(), Rand);
			UE_LOG(LogTemp, Warning, TEXT("Final Search Location : %s"), *SearchLocation.ToString());
		}

		MoveResult = MoveToLocation(SearchLocation);
		GetWorldTimerManager().SetTimer(MoveTimer, 1.0, true);
		
		UE_LOG(LogTemp, Warning, TEXT("MoveTimer is Set And Wait Move request."));

		while (!(GetWorldTimerManager().IsTimerPaused(MoveTimer)))
		{
			if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal || MoveResult == EPathFollowingRequestResult::Failed)
			{
				UE_LOG(LogTemp, Warning, TEXT("Move Request is success And Pause Timer"));
				GetWorldTimerManager().PauseTimer(MoveTimer);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Finally Timer is Clear"));
		GetWorldTimerManager().ClearTimer(MoveTimer);
	}
}*/

/***************************************/
//////////// Behavior Tree //////////////
/***************************************/
void AEnemyAIController::OnPossess(APawn * InPawn)
{
	Super::OnPossess(InPawn);

	AEnemy* Enemy = Cast<AEnemy>(InPawn);
	if (Enemy && Enemy->EnemyBehavior)
	{
		BBComp->InitializeBlackboard(*(Enemy->EnemyBehavior->BlackboardAsset)); //Blackboard초기화.
		BBComp->SetValueAsVector(OriginPosKey, Enemy->GetActorLocation()); //월드에 스폰한 위치를 저장



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