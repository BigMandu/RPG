// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "EnemyAIController.h"
#include "MainCharacter.h"
#include "EnemyAnimInstance.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
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
	
	MaxHealth = 100.f;
	Health = 100.f;
	Damage = 10.f;
	AttackRange = 100.f;
	AttackRadius = 50.f;

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
	AnimInstance = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance());
	check(AnimInstance != nullptr);

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapEnd);
	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlapEnd);

	//AnimInstance->OnMontageEnded.AddDynamic(this, &AEnemy::OnCombatMontageEnded);
	AnimInstance->RangeAttack.AddUObject(this, &AEnemy::AttackGiveDamage);
	AnimInstance->AttackEnd.AddUObject(this, &AEnemy::AttackEnd);


	AttackRange = CombatSphere->GetScaledSphereRadius() * 1.25f;
	AttackRadius = 45.f;
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


	//CanAttack�̸� Target(Player)�� ���ϵ��� ȸ���� �߰��Ѵ�.
	if (AIController->GetBlackboardComponent()->GetValueAsBool(AIController->CanAttackKey) ||
		AIController->GetBlackboardComponent()->GetValueAsBool(AIController->CanDashAttackKey))
	{
		if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attack) //������ ����������, ������ ���� �������� ȸ���ϵ��� ����.
		{
			RotateToTarget();
		}
	}
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}



	//////////////////////////////
	/****    Enemy Combat    ****/
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
	/*
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar && AIController)
		{
			AIController->UpdateCanDashAttack(true);
		}
	}*/
}

void AEnemy::AgroSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	/*
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar && AIController)
		{
			AIController->UpdateCanDashAttack(false);
		}
	}*/
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
		}
	}
}

bool AEnemy::ReturnHit() //AnimNotifyState_RangeAttack()���� ȣ����.
{
	if (bWasHit == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy::ReturnHit() return True"));
		return true;
	}
	
	AttackGiveDamage();
	UE_LOG(LogTemp, Warning, TEXT("Enemy::ReturnHit() return False"));
	return false;
}

void AEnemy::AttackGiveDamage()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy::AttackGiveDamage Function called!"));

	FHitResult HitResult;
	FVector StartVec = GetActorLocation();
	FVector EndVec = GetActorForwardVector() * AttackRange + StartVec;
	FCollisionQueryParams QueryParam(FName(TEXT("AttackTraceSingle")), false, this);

	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, StartVec, EndVec, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(AttackRadius), QueryParam);

	/*������*/
	FColor Color = bHit ? FColor::Green : FColor::Red;
	DrawDebugCapsule(GetWorld(), GetActorLocation() + GetActorForwardVector() * AttackRange * 0.5,
		AttackRange * 0.5 + AttackRadius, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector() * AttackRange).ToQuat(),
		Color, false, 2.0f);

	if (bHit)
	{
		if (HitResult.Actor.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT(" Hit Actor name is : %s"), *HitResult.GetActor()->GetName());

			AMainCharacter* Main = Cast<AMainCharacter>(HitResult.GetActor());
			if (Main)
			{
				UGameplayStatics::ApplyDamage(Main, Damage, GetController(), this, DamageTypeClass);
				UE_LOG(LogTemp, Warning, TEXT("Enemy::ApplyDamage"));
				UE_LOG(LogTemp, Warning, TEXT("Damage : %f"), Damage);
			}
		}
		bWasHit = true;
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy::TakeDamage()::call Die()!!:::::Damage Causer : %s , Damage : %f"), *DamageCauser->GetName(), DamageAmount);
		Die();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy::TakeDamage()::::Damage Causer : %s , Damage : %f"), *DamageCauser->GetName(), DamageAmount);
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AEnemy::Die()
{
	Health = 0;
	UE_LOG(LogTemp, Warning, TEXT("Enemy::Die()"));
}

void AEnemy::Attack(UBlackboardComponent* BBComp)
{	
	if (!bAttacking)
	{
		bAttacking = true;
		if (AnimInstance && AIController)
		{
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attack);

			bool CloseCombat = BBComp->GetValueAsBool(AIController->CanAttackKey); //���������� ����ؾ��ϴ���
			bool DashAttack = BBComp->GetValueAsBool(AIController->CanDashAttackKey); //�뽬������ �ؾ��ϴ��� �Ǵ� �ϱ� ���� bb���� ���� ������.
			

			if (CloseCombat && !DashAttack) //���������� true��
			{
				//bAttacking = true;
				//AIController->StopMovement();
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
			
			if (!CloseCombat && DashAttack) //Dash������ true��
			{

				FVector CurrentVector = GetActorLocation();
				FVector DashVector(GetActorForwardVector() * 1200.f);
				DashVector.Z = CurrentVector.Z; //��������, �������濡�� ��� ����??

				UE_LOG(LogTemp, Warning, TEXT("DashVector is : %s"), *DashVector.ToString());
				
				//AIController->MoveToLocation(DashVector + BeforeVector);
				GetCharacterMovement()->MaxWalkSpeed = 950.f;

				AnimInstance->Montage_Play(DashAttackCombatMontage, 1.2f);
				
				GetWorld()->GetTimerManager().SetTimer(DashAttackHandle, [=]
				{
					if (FName("Dash") == AnimInstance->Montage_GetCurrentSection(DashAttackCombatMontage))
					{
						RotateToTarget(); //�뽬�� ȸ���� �����ϰ� �ѹ� �־�ô�.
						//bAttacking = true;
						FAIMoveRequest MoveReq;
						MoveReq.SetGoalLocation(DashVector + CurrentVector);
						MoveReq.SetStopOnOverlap(true); //�߰��غ�.
						AIController->MoveTo(MoveReq);
					}
				}, 1.0f, true);
			}
		}
	}
}


//���� ����� ������, OnMontageEnded�� ȣ��Ǵµ�, �̶� delegate�� bind�� �� �Լ��� ȣ��Ǹ鼭
//bAttacking�� false������.
//void AEnemy::OnCombatMontageEnded(UAnimMontage* Montage, bool bInterrupted)
//{
//	bAttacking = false;
//	if (DashAttackHandle.IsValid())
//	{
//		GetWorldTimerManager().ClearTimer(DashAttackHandle);
//	}
//}

void AEnemy::AttackEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("Delegate:: Enemy:: Attack End Function call"));
	bAttacking = false;
	bWasHit = false; //ReturnHit()���� �����.
	if (DashAttackHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(DashAttackHandle);
	}
}


void AEnemy::RotateToTarget()
{
	float InterpSpeed = 5.0f;
	UBlackboardComponent* BBComp = AIController->GetBlackboardComponent();
	AMainCharacter* MainChar = Cast<AMainCharacter>(BBComp->GetValueAsObject(AIController->TargetKey));
	if (!MainChar || !BBComp)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Enemy->RotateToTarget() Failed!"));
		return;
	}
	FVector TargetLo = MainChar->GetActorLocation();
	FVector OriginLo = GetActorLocation();
	
	FVector LookAtVector = (TargetLo - OriginLo).GetSafeNormal(); //���⺤��
	FRotator LookAtYaw(0.f, (LookAtVector.Rotation()).Yaw, 0.f);

	//UE_LOG(LogTemp, Warning, TEXT("PlayerLocation : %s"), *TargetLo.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("LookVector : %s"), *LookAtVector.ToString());

	FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, GetWorld()->GetDeltaSeconds(), InterpSpeed);
	
	SetActorRotation(InterpRotation);
}