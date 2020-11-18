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
	VisionAngleDegrees = 50.f;
	MaxAge = 20.f;
	SenseSightConfig->SightRadius = SightRadius;
	SenseSightConfig->LoseSightRadius = LoseSightradius;
	SenseSightConfig->PeripheralVisionAngleDegrees = VisionAngleDegrees; //AI�� �þ߰� ����
	SenseSightConfig->SetMaxAge(MaxAge);
	SenseSightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	//////////////////Behavior Tree//////////////////

	BTComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BTComp"));
	BBComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BBComp"));
	
}

void AEnemyAIController::BeginPlay() //������ ���۵ɶ� ȣ���.
{
	Super::BeginPlay();
	
	//MoveRandom test, timerhandle, navsys
	/*
	FTimerHandle RandomTimerHandle;
	GetWorldTimerManager().SetTimer(RandomTimerHandle, this, &AEnemyAIController::MoveToRandomLocation, 3.0f, true);
	
	//NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	*/

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::DetectActor);
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AEnemyAIController::DetectActor(AActor* Actor, FAIStimulus Stimulus)
{
	FVector DetectLo = Stimulus.StimulusLocation; //������ ��ġ�� ����.

	if (Actor)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(Actor); //Main���� ĳ��Ʈ.
		if (Main)
		{
			if (Stimulus.WasSuccessfullySensed()) //���������� ������ ������
			{
				GetWorldTimerManager().ClearTimer(LostTargetTimer); //Timer�ʱ�ȭ.

				//���������� Target Key�� HasDetected�� Update����.
				UpdateTargetKey(Actor);
				UpdateHasDetectedPlayer(true);

				//�Ʒ��� �����.
				{
					UKismetSystemLibrary::DrawDebugSphere(this, FVector(DetectLo.X, DetectLo.Y, DetectLo.Z + 70.f), 50.f, 12, FLinearColor::Green, 3.f, 2.f);
					UE_LOG(LogTemp, Warning, TEXT("AI : Detected!! / Target : %s, Location : %s" ), *(Actor->GetName()), *DetectLo.ToString());
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Detected Location : %s"), *DetectLo.ToString()));
					}
				}
			}
			else //������ ���Ѱ��
			{
				LostTargetDelegate = FTimerDelegate::CreateUObject(this, &AEnemyAIController::TargetLost, Actor); //TimerDelegate�� �̿��ؼ� �Ķ���͸� �Ѱ���
				GetWorldTimerManager().SetTimer(LostTargetTimer, LostTargetDelegate, 5.0f, false); //SetTimer�� �Լ��� ȣ��

				UpdateHasDetectedPlayer(false);
				
				//�����
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
	Actor = nullptr; //Main�� null�� �ٲ���
	
	UpdateTargetKey(nullptr);
	UpdateHasDetectedPlayer(false);

	//�Ʒ��� �����
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetLost() // AI : Target Lost!!"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("TargetLost() // Target Lost!!")));
		}
	}
}

/*
void AEnemyAIController::MoveToRandomLocation() //TEST����
{
	//AEnemy* Enemy = GetWorld()->SpawnActor<AEnemy>(); //Enemy�� ��� SapwnActor�� �ϱ⶧���� NULL�� ���涧�� �ִ�.
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	if (NavSys)//NavSystem)
	{
		FVector Origin = FVector::ZeroVector;
		FVector RandomLocation = FVector(NavSys->GetRandomReachablePointInRadius(this, Origin, 2500.f));

		//NavSystem->SimpleMoveToLocation(AIController, LinearRandLocation);
		MoveToLocation(RandomLocation);//LinearRandLocation);

		//�����
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

	AEnemy* Enemy = Cast<AEnemy>(InPawn);
	if (Enemy && Enemy->EnemyBehavior)
	{
		BBComp->InitializeBlackboard(*(Enemy->EnemyBehavior->BlackboardAsset)); //Blackboard�ʱ�ȭ.
		BBComp->SetValueAsVector(OriginPosKey, Enemy->GetActorLocation());



		BTComp->StartTree(*(Enemy->EnemyBehavior)); //���ϸ������� StartTree�� �ؾ���.
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't initialize Blackboard"));
	}

	/*
	if (UseBlackboard(BBAsset, Blackboard)) //blackboard Asset�� Component�� ���������� ����ȴٸ� true return.
	{
		if (!RunBehaviorTree(BTAsset)) //BehaviorTree�� �������� ���ϸ�,
		{
			UE_LOG(LogTemp, Warning, TEXT("behavior Tree�� ������ �� ����"));
		}
		Blackboard->SetValueAsVector(OriginPosKey, InPawn->GetActorLocation()); //OriginPosKey�� ���� Enemy�� ��ġ���� �޾ƿ�.
	}*/


}


/************    Blackboard Key update function   ***********/

//Target Object update �Լ�.
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