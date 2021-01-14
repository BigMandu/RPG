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
	SenseSightConfig->PeripheralVisionAngleDegrees = VisionAngleDegrees; //AI�� �þ߰� ����
	SenseSightConfig->SetMaxAge(MaxAge);
	SenseSightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	//////////////////Behavior Tree//////////////////

	BTComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BTComp"));
	BBComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BBComp"));

	BBComp->SetValueAsVector(LastPlayerLocationKey, FVector::ZeroVector); //BehaviorTree�� Invalid��� ���ͼ� �׳� �ʱ�ȭ�� �����. 
	
}

void AEnemyAIController::BeginPlay() //������ ���۵ɶ� ȣ���.
{
	Super::BeginPlay();

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::DetectActor);
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Dash Attack(Dash Attack) ���ɿ��� �Ǵ� -> DashAttack Animation�� ������ ����.
	if (Enemy->DashAttackCombatMontage)
	{
		if (BBComp->GetValueAsBool(HasDetectedPlayerKey))
		{
			AMainCharacter* MainChar = Cast<AMainCharacter>(BBComp->GetValueAsObject(TargetKey));
			if (MainChar)
			{
				FVector TargetLo = MainChar->GetActorLocation();
				float DistanceToTarget = FVector::Dist(TargetLo, Enemy->GetActorLocation());
				if (DistanceToTarget >= SightRadius - 200.f) //���� && �Ÿ�= (�þ߹��� - 200.f) �̻��̸� dashattack����.
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


	//��÷� Enum������Ʈ�ϱ� ���ؼ�.
	UpdateEnumMovementStatus(Enemy->EnemyMovementStatus);
}


void AEnemyAIController::DetectActor(AActor* Actor, FAIStimulus Stimulus)
{
	FVector DetectLo = Stimulus.StimulusLocation; //������ ��ġ�� ����.
	float DetectedPlayerLostTime = 7.0f; //Target�� �ʱ�ȭ �ϴ� �ð�.

	if (Actor)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(Actor); //Main���� ĳ��Ʈ.
		if (Main)
		{
			if (Main->MovementStatus == EMovementStatus::EMS_Dead) //Target�� �׾������� �������� �ʵ��� �Ѵ�.
				return;
			if (Stimulus.WasSuccessfullySensed()) //���������� ������ ������
			{
				GetWorldTimerManager().ClearTimer(LostTargetTimer); //Timer�ʱ�ȭ.

				//���������� Target Key�� HasDetected�� Update����.
				UpdateTargetKey(Main);
				UpdateHasDetectedPlayer(true);

				Enemy->EnemyHPbarComp->SetVisibility(true); //Enemy�� Player�� �ĺ������� Enemy HP bar�� �����ش�.

				//�Ʒ��� �����.
				{
					//UKismetSystemLibrary::DrawDebugSphere(this, FVector(DetectLo.X, DetectLo.Y, DetectLo.Z + 70.f), 50.f, 12, FLinearColor::Green, 3.f, 2.f);
					////UE_LOG(LogTemp, Warning, TEXT("AI : Detected!! / Target : %s, Location : %s" ), *(Main->GetName()), *DetectLo.ToString());
					//if (GEngine)
					//{
					//	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Detected Location : %s"), *DetectLo.ToString()));
					//}
				}
			}
			else //������ ���Ѱ��
			{
				LostTargetDelegate = FTimerDelegate::CreateUObject(this, &AEnemyAIController::TargetLost, Actor); //TimerDelegate�� �̿��ؼ� �Ķ���͸� �Ѱ���
				GetWorldTimerManager().SetTimer(LostTargetTimer, LostTargetDelegate, DetectedPlayerLostTime, false); //SetTimer�� �Լ��� ȣ��
				
				BBComp->SetValueAsRotator(LastPlayerRotationKey, Main->GetActorRotation());// Player�� ȸ������
				BBComp->SetValueAsVector(LastPlayerLocationKey, DetectLo); //������ ���� ��ġ�� Blackboard�� �Ѱ��ش�.
				UpdateHasDetectedPlayer(false);
				
				//�����
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
		//Actor = nullptr; //Main�� null�� �ٲ���

		UpdateTargetKey(nullptr);
		UpdateHasDetectedPlayer(false);

		
		Enemy->EnemyHPbarComp->SetVisibility(false); //Enemy�� Player�� ������ ��ġ�� HPbar�� �����.

		//�Ʒ��� �����
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
		BBComp->InitializeBlackboard(*(Enemy->EnemyBehavior->BlackboardAsset)); //Blackboard�ʱ�ȭ.
		BBComp->SetValueAsVector(OriginPosKey, Enemy->GetActorLocation()); //���忡 ������ ��ġ�� ����



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

void AEnemyAIController::UpdateCanAttack(bool CanAttack)
{
	BBComp->SetValueAsBool(CanAttackKey, CanAttack);
}

void AEnemyAIController::UpdateCanDashAttack(bool CanDashAttack) //EnemyAI -> Tick���� �����.
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