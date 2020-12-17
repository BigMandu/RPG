// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "EnemyAIController.h"
#include "MainCharacter.h"
#include "EnemyAnimInstance.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "EnemyWidget.h"
#include "Soul.h"


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


	//UI Enemy HP bar
	EnemyHPbarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHPbar"));
	EnemyHPbarComp->SetupAttachment(GetMesh());
	EnemyHPbarComp->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	EnemyHPbarComp->SetWidgetSpace(EWidgetSpace::Screen);
	

	//경로를 받아오는 방법..별로 안좋은것 같아서 안쓴다.
	/*static ConstructorHelpers::FClassFinder<UUserWidget> UI_HUD(TEXT("/Game/Blueprints/HUD/EnemyHPbar.EnemyHPbar_C"));
	if (UI_HUD.Succeeded())
	{
		EnemyHPbar->SetWidget
		EnemyHPbar->SetWidgetClass(UI_HUD.Class);
		EnemyHPbar->SetDrawSize(FVector2D(180.f, 30.f));
	}*/

	//없으면 가져오는건데, 이상하게 WEnemyHealthBar가 nullptr이다.(editor에서 설정을 해줬는데도 nullptr임..)이런적 없었는데

	//Combat 관련 
	NumberOfCombatAnim = 3; //CombatMontage Animation 개수 디폴트값 지정
	bAttacking = false;
	
	MaxHealth = 100.f;
	Health = 100.f;
	Damage = 10.f;
	AttackRange = 100.f;
	AttackRadius = 50.f;

	SoulMin = 1;
	SoulMax = 4;
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


	//Enemy HP BarComponent에 Widgetclass가 없을때
	if (EnemyHPbarComp->GetWidgetClass() == nullptr)
	{
		if (WEnemyHealthBar == nullptr) //만약 TSubclassof EnemyHealthbar에도 지정이 안되어있다면
		{
			UE_LOG(LogTemp, Warning, TEXT("WidgetClass is null,,Setting Widget in Editor or EnemyHPbarComponent"));
		}

		EnemyHPbarComp->SetWidgetClass(WEnemyHealthBar); //WEnemyHealthBar에 넣어둔 UUserwidget으로 세팅함.
	}
	EnemyHPbarComp->SetDrawSize(FVector2D(180.f, 30.f));
	EnemyHPbarComp->SetVisibility(false);

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	//AI Controller로 캐스트,
	AIController = Cast<AEnemyAIController>(GetController());

	EnemyWidget = Cast<UEnemyWidget>(EnemyHPbarComp->GetUserWidgetObject()); //EnmeyHPbarComp에 설정한 Widget을 Cast한다.

	if (EnemyWidget)
	{
		//UE_LOG(LogTemp, Warning, TEXT("EnemyWidget->EnemyMaxHP is : %f"), MaxHealth);
		EnemyWidget->EnemyMaxHP = MaxHealth; //EnemyWidget의 MAX HP값을 넘겨준다.
		EnemyWidget->EnemyHP = Health;
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	//CanAttack이면 Target(Player)에 향하도록 회전을 추가한다.
	if (AIController->GetBlackboardComponent()->GetValueAsBool(AIController->CanAttackKey) ||
		AIController->GetBlackboardComponent()->GetValueAsBool(AIController->CanDashAttackKey))
	{
		if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attack) //공격은 가능하지만, 공격을 하지 않을때만 회전하도록 변경.
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

bool AEnemy::ReturnHit() //AnimNotifyState_RangeAttack()에서 호출함.
{
	if (bWasHit == true)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Enemy::ReturnHit() return True"));
		return true;
	}
	
	AttackGiveDamage();
	//UE_LOG(LogTemp, Warning, TEXT("Enemy::ReturnHit() return False"));
	return false;
}

void AEnemy::AttackGiveDamage()
{
	//UE_LOG(LogTemp, Warning, TEXT("Enemy::AttackGiveDamage Function called!"));

	FHitResult HitResult;
	FVector StartVec = GetActorLocation();
	FVector EndVec = GetActorForwardVector() * AttackRange + StartVec;
	FCollisionQueryParams QueryParam(FName(TEXT("AttackTraceSingle")), false, this);

	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, StartVec, EndVec, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(AttackRadius), QueryParam);

	/*디버깅용*/
	FColor Color = bHit ? FColor::Green : FColor::Red;
	DrawDebugCapsule(GetWorld(), GetActorLocation() + GetActorForwardVector() * AttackRange * 0.5,
		AttackRange * 0.5 + AttackRadius, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector() * AttackRange).ToQuat(),
		Color, false, 2.0f);

	if (bHit)
	{
		if (HitResult.Actor.IsValid())
		{
			//UE_LOG(LogTemp, Warning, TEXT(" Hit Actor name is : %s"), *HitResult.GetActor()->GetName());

			AMainCharacter* Main = Cast<AMainCharacter>(HitResult.GetActor());
			if (Main)
			{
				UGameplayStatics::ApplyDamage(Main, Damage, GetController(), this, DamageTypeClass);
				//UE_LOG(LogTemp, Warning, TEXT("Enemy::ApplyDamage"));
				//UE_LOG(LogTemp, Warning, TEXT("Damage : %f"), Damage);
			}
		}
		bWasHit = true;
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	//인식하지 못했는데 데미지를 받았으면 바로 돌게하기 위함
	{
		AIController->UpdateTargetKey(DamageCauser);
		RotateToTarget();
	}
	if (Health - DamageAmount <= 0.f)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Enemy::TakeDamage()::call Die()!!:::::Damage Causer : %s , Damage : %f"), *DamageCauser->GetName(), DamageAmount);
		Die();
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Enemy::TakeDamage()::::Damage Causer : %s , Damage : %f"), *DamageCauser->GetName(), DamageAmount);
		Health -= DamageAmount;
	}

	if (EnemyWidget)
	{
		//UE_LOG(LogTemp, Warning, TEXT("EnemyWidget->EnemyHP is : %f"), Health);
		EnemyWidget->EnemyHP = Health; //Damage를 받을때 마다 update해준다.
	}

	return DamageAmount;
}

void AEnemy::Die()
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);
	Health = 0.f;
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (Anim)
	{
		Anim->Montage_Play(SpiderHitDeathMontage, 1.2f);
		Anim->Montage_JumpToSection(FName("Death"), SpiderHitDeathMontage);
		
	}
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	
	
	UE_LOG(LogTemp, Warning, TEXT("Enemy::Die()"));
}

void AEnemy::DeathEnd() //Die함수에서 재생하는 Animation의 Notify에서 호출.
{
	FTimerHandle DeathTimer;
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DeathClear, 4.0f);
}

void AEnemy::DeathClear()
{
	SpawnLoot();
	Destroy();
	//Soul의 Spawn을 여기서 한다.
}

void AEnemy::SpawnLoot()
{	

	int32 SoulQuantity = FMath::RandRange(SoulMin, SoulMax);

	UWorld* World = GetWorld();

	UObject* SpawnSoul = Cast<UObject>(StaticLoadObject(UObject::StaticClass(), NULL, TEXT("/Game/Blueprints/Soul_BP.Soul_BP")));
	UBlueprint* SoulClass = Cast<UBlueprint>(SpawnSoul);
	FActorSpawnParameters SpawnParams;
	SpawnItemArea = FVector(40.f);
	/*static ConstructorHelpers::FObjectFinder<UBlueprint> SoulBP(TEXT("Blueprint'/Game/Blueprints/Soul_BP.Soul_BP'"));
	if (SoulBP.Object)
	{
		SoulClass = (UClass*)SoulBP.Object->GeneratedClass;
	}*/

	if (World && SoulClass)
	{
		for (int it =0; it < SoulQuantity; it++)
		{
			const FVector SpawnLocation = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), SpawnItemArea);
			World->SpawnActor<ASoul>(SoulClass->GeneratedClass, SpawnLocation, FRotator(0.f), SpawnParams);
		}
	}
}


void AEnemy::Attack(UBlackboardComponent* BBComp)
{	
	if (!bAttacking)
	{
		bAttacking = true;
		if (AnimInstance && AIController)
		{
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attack);

			bool CloseCombat = BBComp->GetValueAsBool(AIController->CanAttackKey); //근접공격을 재생해야하는지
			bool DashAttack = BBComp->GetValueAsBool(AIController->CanDashAttackKey); //대쉬공격을 해야하는지 판단 하기 위해 bb에서 값을 가져옴.
			

			if (CloseCombat && !DashAttack) //근접공격이 true면
			{
				//bAttacking = true;
				//AIController->StopMovement();
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
			
			if (!CloseCombat && DashAttack) //Dash공격이 true면
			{

				FVector CurrentVector = GetActorLocation();
				FVector DashVector(GetActorForwardVector() * 1200.f);
				DashVector.Z = CurrentVector.Z; //오르막길, 내리막길에선 어떻게 하지??

				//UE_LOG(LogTemp, Warning, TEXT("DashVector is : %s"), *DashVector.ToString());
				
				//AIController->MoveToLocation(DashVector + BeforeVector);
				GetCharacterMovement()->MaxWalkSpeed = 950.f;

				AnimInstance->Montage_Play(DashAttackCombatMontage, 1.2f);
				
				GetWorld()->GetTimerManager().SetTimer(DashAttackHandle, [=]
				{
					if (FName("Dash") == AnimInstance->Montage_GetCurrentSection(DashAttackCombatMontage))
					{
						RotateToTarget(); //대쉬중 회전도 가능하게 한번 넣어봤다.
						//bAttacking = true;
						FAIMoveRequest MoveReq;
						MoveReq.SetGoalLocation(DashVector + CurrentVector);
						MoveReq.SetStopOnOverlap(true); //추가해봄.
						AIController->MoveTo(MoveReq);
					}
				}, 1.0f, true);
			}
		}
	}
}

void AEnemy::AttackEnd()
{
	//UE_LOG(LogTemp, Warning, TEXT("Delegate:: Enemy:: Attack End Function call"));
	bAttacking = false;
	bWasHit = false; //ReturnHit()에서 사용함.
	if (DashAttackHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(DashAttackHandle);
	}
}


void AEnemy::RotateToTarget()
{
	float InterpSpeed = 20.0f;
	UBlackboardComponent* BBComp = AIController->GetBlackboardComponent();
	AMainCharacter* MainChar = Cast<AMainCharacter>(BBComp->GetValueAsObject(AIController->TargetKey));
	if (!MainChar || !BBComp)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Enemy->RotateToTarget() Failed!"));
		return;
	}
	FVector TargetLo = MainChar->GetActorLocation();
	FVector OriginLo = GetActorLocation();
	
	FVector LookAtVector = (TargetLo - OriginLo).GetSafeNormal(); //방향벡터
	FRotator LookAtYaw(0.f, (LookAtVector.Rotation()).Yaw, 0.f);

	//UE_LOG(LogTemp, Warning, TEXT("PlayerLocation : %s"), *TargetLo.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("LookVector : %s"), *LookAtVector.ToString());

	FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, GetWorld()->GetDeltaSeconds(), InterpSpeed);
	
	SetActorRotation(InterpRotation);
}