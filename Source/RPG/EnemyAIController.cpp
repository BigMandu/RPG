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
#include "BehaviorTree/BlackboardData.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SenseSightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
	PerceptionComponent->ConfigureSense(*SenseSightConfig);
	SightRadius = 1000.f;
	LoseSightradius = 1500.f;
	VisionAngleDegrees = 50.f;
	MaxAge = 20.f;
	SenseSightConfig->SightRadius = SightRadius;
	SenseSightConfig->LoseSightRadius = LoseSightradius;
	SenseSightConfig->PeripheralVisionAngleDegrees = VisionAngleDegrees; //AI의 시야각 설정
	SenseSightConfig->SetMaxAge(MaxAge);
	SenseSightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	//////////////////Behavior Tree//////////////////

	static ConstructorHelpers::FObjectFinder<UBlackboardData> BBObject(TEXT("/Game/Blueprints/AI/BB_EnemyAI.BB_EnemyAI"));
	if (BBObject.Succeeded())
	{
		BBAsset = BBObject.Object; //Blackboard를 가져옴
	}
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObject(TEXT("/Game/Blueprints/AI/BT_EnemyAI.BT_EnemyAI"));
	if (BTObject.Succeeded())
	{
		BTAsset = BTObject.Object; //behavior tree를 가져옴.
	}
}

void AEnemyAIController::BeginPlay() //레벨이 시작될때 호출됨.
{
	Super::BeginPlay();
	
	FTimerHandle RandomTimerHandle;
	GetWorldTimerManager().SetTimer(RandomTimerHandle, this, &AEnemyAIController::MoveToRandomLocation, 3.0f, true);

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

	if (Actor)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(Actor); //Main으로 캐스트.
		if (Main)
		{
			if (Stimulus.WasSuccessfullySensed()) //성공적으로 감지를 했으면
			{
				GetWorldTimerManager().ClearTimer(LostTargetTimer); //Timer초기화.
				//아래는 디버깅.
				{
					UKismetSystemLibrary::DrawDebugSphere(this, FVector(DetectLo.X, DetectLo.Y, DetectLo.Z + 70.f), 50.f, 12, FLinearColor::Green, 3.f, 2.f);
					UE_LOG(LogTemp, Warning, TEXT("AI : Detected!! / Location : %s"), *DetectLo.ToString());
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Detected Location : %s"), *DetectLo.ToString()));
					}
				}
			}
			else //감지를 못한경우
			{
				LostTargetDelegate = FTimerDelegate::CreateUObject(this, &AEnemyAIController::TargetLost, Actor); //TimerDelegate를 이용해서 파라미터를 넘겨줌
				GetWorldTimerManager().SetTimer(LostTargetTimer, LostTargetDelegate, 5.0f, false); //SetTimer로 함수를 호출

				//디버깅용
				{
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
	//아래는 디버깅
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetLost() // AI : Target Lost!!"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("TargetLost() // Target Lost!!")));
		}
	}
}

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


/***************************************/
//////////// Behavior Tree //////////////
/***************************************/
void AEnemyAIController::OnPossess(APawn * InPawn)
{
	Super::OnPossess(InPawn);
	if (UseBlackboard(BBAsset, Blackboard))
	{
		if (!RunBehaviorTree(BTAsset))
		{
			UE_LOG(LogTemp, Warning, TEXT("behavior Tree를 실행할 수 없음"));
		}
	}
}
