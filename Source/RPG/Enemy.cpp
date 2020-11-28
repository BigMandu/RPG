// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "EnemyAIController.h"
#include "MainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
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
	AgroSphere->InitSphereRadius(1000.f); //SightRadius�� ���ϰ����� ��������.

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(80.f);

	//�������� ����Ʈ�� 500 ����. BTTask_SearchPatrolLocation���� ���.
	PatrolArea = 500.f; 	

	//������
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f); //���� ȸ���ӵ���.
	bUseControllerRotationYaw = false;

	//AIController�� �������ش�.
	AIControllerClass = AEnemyAIController::StaticClass();
	
	//Pawn�� AI��Ʈ�ѷ��� �����ϰ�, �����ϴ� �ñ⸦ �����Ѵ�.���⼱ World�� ��ġ�ǰų� �����ɶ�.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned; 
	
	//Combat ���� 
	NumberOfCombatAnim = 3; //CombatMontage Animation ���� ����Ʈ�� ����
	bAttacking = false;


	/*
	//////////AI TEST/////////
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SenseSightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
	PerceptionComponent->ConfigureSense(*SenseSightConfig);
	*/
}


void AEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AnimInstance = GetMesh()->GetAnimInstance();

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapEnd);
	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlapEnd);

	AnimInstance->OnMontageEnded.AddDynamic(this, &AEnemy::OnCombatMontageEnded);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	//AI Controller�� ĳ��Ʈ,
	AIController = Cast<AEnemyAIController>(GetController());

	


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
			//AIController->UpdateCanDashAttack(true);
		}
	}
}

void AEnemy::AgroSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar && AIController)
		{
			//AIController->UpdateCanDashAttack(false);
		}
	}
}

void AEnemy::CombatSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar && AIController)
		{
			//SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attack);
			AIController->UpdateCanAttack(true);
			//AIController->UpdateCanDashAttack(false);

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
			AIController->UpdateCanAttack(false);
			//AIController->UpdateCanDashAttack(true);
		}
	}
}


void AEnemy::Attack(UBlackboardComponent* BBComp)
{	
	
	if (!bAttacking)
	{
		bAttacking = true;
		//UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); �����ڿ� ������.
		AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController());
		
		if (AnimInstance && AICon)
		{
			//AnimInstance->OnMontageEnded.AddDynamic(this, &AEnemy::OnCombatMontageEnded); //OnMontageEnded�� Custom�� ��������. =-> postinitial������.
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attack);
			bool CloseCombat = BBComp->GetValueAsBool(AICon->CanAttackKey); //���������� ����ؾ��ϴ���
			bool DashAttack = BBComp->GetValueAsBool(AICon->CanDashAttackKey); //�뽬������ �ؾ��ϴ��� �Ǵ� �ϱ� ���� bb���� ���� ������.

			if (CloseCombat)// && !DashAttack) //���������� true��
			{
				AICon->StopMovement();
				Section = FMath::RandRange(0, NumberOfCombatAnim - 1); //�������� in32Ÿ���� ����.
				AnimInstance->Montage_Play(CloseCombatMontage, 1.25f);

				switch (Section)
				{
				case 0:
					AnimInstance->Montage_JumpToSection(FName("Attack_1"), CloseCombatMontage);
					break;
				case 1:
					AnimInstance->Montage_JumpToSection(FName("Attack_2"), CloseCombatMontage);
					break;
				case 2:
					AnimInstance->Montage_JumpToSection(FName("Attack_Strong"), CloseCombatMontage);
					break;
				default:
					break;
				}
			}
			//Dash Attack...����...
			/*
			if (!CloseCombat && DashAttack) //Dash������ true��
			{
				//FTimerHandle Handle;
				//FVector TargetLo = Cast<AMainCharacter>(BBComp->GetValueAsObject(AICon->TargetKey))->GetActorLocation();
				//FVector NormalVector = (TargetLo - GetActorLocation()).GetSafeNormal();
				FVector DashVector = GetActorForwardVector() * 3000.f;//NormalVector * 1000.f;
				//DashVector = FVector(DashVector.X, DashVector.Y, 0.f);


				//GetCharacterMovement()->RequestDirectMove(DashVector, false);
				AnimInstance->Montage_Play(DashAttackCombatMontage, 1.2f);
				AnimInstance->Montage_JumpToSection(FName("Dash"), DashAttackCombatMontage);
				LaunchCharacter(DashVector, true , false);
				//AddMovementInput(DashVector);
				
				
				//GetWorld()->GetTimerManager().SetTimer(Handle, [=] 
				//	{
				//		if (FName("Dash") == AnimInstance->Montage_GetCurrentSection(DashAttackCombatMontage))
				//		{
				//			UE_LOG(LogTemp, Warning, TEXT("MainCharLocation : %s "), *TargetLo.ToString());
				//			UE_LOG(LogTemp, Warning, TEXT("EnemyLocation : %s "), *GetActorLocation().ToString());
				//			UE_LOG(LogTemp, Warning, TEXT("NormalVector : %s, DashVector : %s"), *NormalVector.ToString(), *DashVector.ToString());
				//			//GetCharacterMovement()->RequestDirectMove(DashVector, false);
				//			//GetCharacterMovement()->Velocity = DashVector;
				//			//GetCharacterMovement()->MaxWalkSpeed = 650.f;
				//			LaunchCharacter(DashVector, false, false);
				//		}
				//	}, 1.0f, true);
			}*/
		}
	}
}


//���� ����� ������, OnMontageEnded�� ȣ��Ǵµ�, �̶� delegate�� bind�� �� �Լ��� ȣ��Ǹ鼭
//bAttacking�� false������.
void AEnemy::OnCombatMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bAttacking = false;
}


void AEnemy::RotateToTarget(UBlackboardComponent* BBComp, AEnemyAIController* AICon)
{
	float InterpSpeed = 2.0f;
	FVector TargetLo = Cast<AMainCharacter>(BBComp->GetValueAsObject(AICon->TargetKey))->GetActorLocation();
	FVector OriginLo = GetActorLocation();
	
	FVector LookAtVector = FVector(TargetLo - OriginLo).GetSafeNormal();
	//FRotator LookAtRotation = LookAtVector.Rotation();

	FRotator LookAtRotation = FRotationMatrix::MakeFromX(LookAtVector).Rotator();

	//FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
	
	SetActorRotation(InterpRotation);
}