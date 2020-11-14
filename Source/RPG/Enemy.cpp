// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "EnemyAIController.h"
#include "MainCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"
/*
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
*/
// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(80.f);



	//AIController�� �������ش�.
	AIControllerClass = AEnemyAIController::StaticClass();
	
	//Pawn�� AI��Ʈ�ѷ��� �����ϰ�, �����ϴ� �ñ⸦ �����Ѵ�.���⼱ World�� ��ġ�ǰų� �����ɶ�.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned; 
	

	/*
	//////////AI TEST/////////
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SenseSightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
	PerceptionComponent->ConfigureSense(*SenseSightConfig);
	*/
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	//AI Controller�� ĳ��Ʈ,
	AIController = Cast<AEnemyAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapEnd);
	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlapEnd);


	/*
	
	////////////AI TEST///////////////
	NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	
	//FTimerHandle TimerHandle;
	//GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemy::MoveToRandomLocation, 1.5f, true);
	
	//Perception Sight�߰�.
	SenseSightConfig->SightRadius = 1000.f;
	SenseSightConfig->LoseSightRadius = 1500.f;
	SenseSightConfig->PeripheralVisionAngleDegrees = 50.f; //AI�� �þ߰� ����
	SenseSightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SenseSightConfig->SetMaxAge(30.f);
	PerceptionComponent->ConfigureSense(*SenseSightConfig); //Sight sense�� �־��ش�.	
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemy::DetectActor); //OntargetPerceptionUpdated�̺�Ʈ�� �����Լ��� bind��Ų��.
	*/
}

/*
void AEnemy::MoveToRandomLocation() //TEST����
{
	FVector RandomLocation = FVector(NavSystem->GetRandomReachablePointInRadius(this, GetActorLocation(), 2500.f));
	FVector LinearRandLocation(RandomLocation.X, RandomLocation.Y, GetActorLocation().Z);
	if (NavSystem)
	{
		//NavSystem->SimpleMoveToLocation(AIController, LinearRandLocation);
		AIController->MoveToLocation(LinearRandLocation);
		
		//�����
		{
			UKismetSystemLibrary::DrawDebugSphere(this, LinearRandLocation, 25.f, 6, FLinearColor::Blue, 3.f, 2.f);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Next Location : %s"), *LinearRandLocation.ToString()));
			}
		}
	}
}
/*
void AEnemy::DetectActor(AActor* Actor, FAIStimulus Stimulus)
{
	//FTimerHandle LostTimer;
	//FTimerDelegate LostDelegate;
	FVector DetectLo = Stimulus.StimulusLocation; //������ ��ġ�� ����.

	if (Actor)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(Actor); //Main���� ĳ��Ʈ.
		if (Main)
		{
			if (Stimulus.WasSuccessfullySensed()) //���������� ������ ������
			{
				GetWorldTimerManager().ClearTimer(LostTimer); //Timer�ʱ�ȭ.
				//�Ʒ��� �����.
				{
					UKismetSystemLibrary::DrawDebugSphere(this, FVector(DetectLo.X, DetectLo.Y, DetectLo.Z + 70.f), 50.f, 12, FLinearColor::Green, 3.f, 2.f);
					UE_LOG(LogTemp, Warning, TEXT("AI : Detected!! / Location : %s"), *DetectLo.ToString());
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Detected Location : %s"), *DetectLo.ToString()));
					}
				}
			}
			else //������ ���Ѱ��
			{			
				LostDelegate = FTimerDelegate::CreateUObject(this, &AEnemy::TargetLost, Actor); //TimerDelegate�� �̿��ؼ� �Ķ���͸� �Ѱ���
				GetWorldTimerManager().SetTimer(LostTimer, LostDelegate, 5.0f, false); //SetTimer�� �Լ��� ȣ��
				
				//������
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

void AEnemy::TargetLost(AActor* Actor)
{
	Actor = nullptr; //Main�� null�� �ٲ���
	//�Ʒ��� �����
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetLost() // AI : Target Lost!!"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("TargetLost() // Target Lost!!")));
		}
	}
}
*/

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

	//////////////////////////////
	/****      Enemy AI      ****/
	//////////////////////////////
/*
void AEnemy::Chase(class AMainCharacter* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Chase);
	
	if (AIController)
	{
		//UE_LOG(LogTemp, Warning, TEXT("EnemyMovement : Chase "));
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(15.f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

	}
}*/

void AEnemy::AgroSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("AItem::OnOverlap Begin"));
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar && AIController)
		{
			AIController->Chase(this, MainChar);
		}
	}
}

void AEnemy::AgroSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
}

void AEnemy::CombatSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar)
		{
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attack);
		}
	}
	
}

void AEnemy::CombatSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar && AIController)
		{
			AIController->Chase(this, MainChar);
		}
	}
}
