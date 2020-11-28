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
	AgroSphere->InitSphereRadius(1000.f); //SightRadius와 동일값으로 설정해줌.

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(80.f);

	//정찰범위 디폴트로 500 설정. BTTask_SearchPatrolLocation에서 사용.
	PatrolArea = 500.f; 	

	//움직임
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f); //위의 회전속도값.
	bUseControllerRotationYaw = false;

	//AIController를 지정해준다.
	AIControllerClass = AEnemyAIController::StaticClass();
	
	//Pawn이 AI컨트롤러를 생성하고, 소유하는 시기를 결정한다.여기선 World에 배치되거나 스폰될때.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned; 
	
	//Combat 관련 
	NumberOfCombatAnim = 3; //CombatMontage Animation 개수 디폴트값 지정
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
	
	//AI Controller로 캐스트,
	AIController = Cast<AEnemyAIController>(GetController());

	


	/*
	
	////////////AI TEST///////////////
	NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	
	//FTimerHandle TimerHandle;
	//GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemy::MoveToRandomLocation, 1.5f, true);
	
	//Perception Sight추가.
	SenseSightConfig->SightRadius = 1000.f;
	SenseSightConfig->LoseSightRadius = 1500.f;
	SenseSightConfig->PeripheralVisionAngleDegrees = 50.f; //AI의 시야각 설정
	SenseSightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SenseSightConfig->SetMaxAge(30.f);
	PerceptionComponent->ConfigureSense(*SenseSightConfig); //Sight sense를 넣어준다.	
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemy::DetectActor); //OntargetPerceptionUpdated이벤트를 정의함수에 bind시킨다.
	*/
}

/*
void AEnemy::MoveToRandomLocation() //TEST목적
{
	FVector RandomLocation = FVector(NavSystem->GetRandomReachablePointInRadius(this, GetActorLocation(), 2500.f));
	FVector LinearRandLocation(RandomLocation.X, RandomLocation.Y, GetActorLocation().Z);
	if (NavSystem)
	{
		//NavSystem->SimpleMoveToLocation(AIController, LinearRandLocation);
		AIController->MoveToLocation(LinearRandLocation);
		
		//디버깅
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
	FVector DetectLo = Stimulus.StimulusLocation; //감지된 위치를 저장.

	if (Actor)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(Actor); //Main으로 캐스트.
		if (Main)
		{
			if (Stimulus.WasSuccessfullySensed()) //성공적으로 감지를 했으면
			{
				GetWorldTimerManager().ClearTimer(LostTimer); //Timer초기화.
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
				LostDelegate = FTimerDelegate::CreateUObject(this, &AEnemy::TargetLost, Actor); //TimerDelegate를 이용해서 파라미터를 넘겨줌
				GetWorldTimerManager().SetTimer(LostTimer, LostDelegate, 5.0f, false); //SetTimer로 함수를 호출
				
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

void AEnemy::TargetLost(AActor* Actor)
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
		//UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); 생성자에 생성함.
		AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController());
		
		if (AnimInstance && AICon)
		{
			//AnimInstance->OnMontageEnded.AddDynamic(this, &AEnemy::OnCombatMontageEnded); //OnMontageEnded와 Custom을 연결해줌. =-> postinitial에선언.
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attack);
			bool CloseCombat = BBComp->GetValueAsBool(AICon->CanAttackKey); //근접공격을 재생해야하는지
			bool DashAttack = BBComp->GetValueAsBool(AICon->CanDashAttackKey); //대쉬공격을 해야하는지 판단 하기 위해 bb에서 값을 가져옴.

			if (CloseCombat)// && !DashAttack) //근접공격이 true면
			{
				AICon->StopMovement();
				Section = FMath::RandRange(0, NumberOfCombatAnim - 1); //버림으로 in32타입을 구함.
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
			//Dash Attack...보류...
			/*
			if (!CloseCombat && DashAttack) //Dash공격이 true면
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


//공격 모션이 끝나면, OnMontageEnded가 호출되는데, 이때 delegate로 bind한 이 함수도 호출되면서
//bAttacking을 false시켜줌.
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