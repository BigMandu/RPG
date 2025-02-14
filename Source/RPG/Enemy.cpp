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
#include "Weapon.h"


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
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(80.f);
	CombatSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
	
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

	//RangeAttack이 없는 Enemy는 false로 한다. -> 함수 delegate bind시 에러나기 때문.
	bHasRangeAttack = false; //이 값은 Editor에서 설정한다.
	
	bStrongAttack = false; //강공격을 하는지 설정. Damage를 더 주기 위함. -> Attack, AttackGiveDamage에서 사용
}


void AEnemy::PostInitializeComponents() //여기다가 하ㅓ면 좀 별로인듯... 만들질 못하네 ㅋㅋ
{
	Super::PostInitializeComponents();
	//AnimInstance = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance());
	////check(AnimInstance != nullptr);

	//AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapBegin);
	//AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapEnd);
	//CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlapBegin);
	//CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlapEnd);

	////AnimInstance->OnMontageEnded.AddDynamic(this, &AEnemy::OnCombatMontageEnded);

	//AnimInstance->RangeAttack.AddUObject(this, &AEnemy::AttackRangeDamage);
	//AnimInstance->AttackEnd.AddUObject(this, &AEnemy::AttackEnd);


	//AttackRange = CombatSphere->GetScaledSphereRadius() * 1.25f;
	//AttackRadius = 45.f;

	//
	////Enemy HP BarComponent에 Widgetclass가 없을때
	//if (EnemyHPbarComp->GetWidgetClass() == nullptr)
	//{
	//	if (WEnemyHealthBar == nullptr) //만약 TSubclassof EnemyHealthbar에도 지정이 안되어있다면
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("WidgetClass is null,,Setting Widget in Editor or EnemyHPbarComponent"));
	//	}

	//	EnemyHPbarComp->SetWidgetClass(WEnemyHealthBar); //WEnemyHealthBar에 넣어둔 UUserwidget으로 세팅함.
	//}
	//EnemyHPbarComp->SetDrawSize(FVector2D(180.f, 30.f));
	//EnemyHPbarComp->SetVisibility(false);

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	//여기부터 아래까지는 임시로 해놓은것임. ->Enemy추가를 위함. ->추가 다하면  PostInitializeComponents 주석 해제하고, 아래 코드는 삭제하기.
	AnimInstance = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance());
	check(AnimInstance != nullptr);

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlapEnd);
	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlapEnd);

	AnimInstance->RangeAttack.AddUObject(this, &AEnemy::AttackRangeDamage);
	AttackRange = CombatSphere->GetScaledSphereRadius() * 1.25f;
	AttackRadius = 45.f;

	AnimInstance->AttackEnd.AddUObject(this, &AEnemy::AttackEnd);

	AnimInstance->ActivateCollision.AddUObject(this, &AEnemy::ActivateCollision);
	AnimInstance->DeactivateCollision.AddUObject(this, &AEnemy::DeactivateCollision);


	//카메라 무시
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Overlap);
	

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
	//위부터 여기 까지는 임시로 해놓은것임.

	//AI Controller로 캐스트,
	AIController = Cast<AEnemyAIController>(GetController());


	//Sweepbysinglechannel을 사용하지 않는 Enemy의 행동트리(Dash Attack)을 막기위함. -> Dash Attack montage를 판정하기때문에 필요없어짐.
	//if (bHasRangeAttack == true) //Editor에서 설정하는 bool값.
	//{
	//	AIController->UpdateHasRangeAttack(true);
	//}

	//AIController->UpdateHasRangeAttack(false);


	if (EnemyWeaponType == EEnemyWeaponType::EWT_Weapon)
	{
		if (EnemyWeapon)
		{
			const USkeletalMeshSocket* LeftSocket = GetMesh()->GetSocketByName("weapon_left");
			const USkeletalMeshSocket* RightSocket = GetMesh()->GetSocketByName("weapon_right");
			if (LeftSocket)
			{
				LeftWeapon = GetWorld()->SpawnActor<AWeapon>(EnemyWeapon);
				if (LeftWeapon)
				{
					LeftWeapon->Equip(this, LeftSocket);
					LeftWeapon->SetWeaponOwner(this);
				}
				
			}
			if (RightSocket)
			{
				RightWeapon = GetWorld()->SpawnActor<AWeapon>(EnemyWeapon);
				if (RightWeapon)
				{
					RightWeapon->Equip(this, RightSocket);
					RightWeapon->SetWeaponOwner(this);
				}
			}
		}
	}


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

	CurrentMovementSpeed = GetCharacterMovement()->GetMaxSpeed();

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
	
	AttackRangeDamage();
	//UE_LOG(LogTemp, Warning, TEXT("Enemy::ReturnHit() return False"));
	return false;
}


void AEnemy::AttackRangeDamage() //범위공격
{
	//UE_LOG(LogTemp, Warning, TEXT("Enemy::AttackGiveDamage Function called!"));

	FHitResult HitResult;
	FVector StartVec = GetActorLocation();
	FVector EndVec = GetActorForwardVector() * AttackRange + StartVec;
	FCollisionQueryParams QueryParam(FName(TEXT("AttackTraceSingle")), false, this);

	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, StartVec, EndVec, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(AttackRadius), QueryParam);

	/*디버깅용*/
	/*FColor Color = bHit ? FColor::Green : FColor::Red;
	DrawDebugCapsule(GetWorld(), GetActorLocation() + GetActorForwardVector() * AttackRange * 0.5,
		AttackRange * 0.5 + AttackRadius, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector() * AttackRange).ToQuat(),
		Color, false, 2.0f);*/

	if (bHit)
	{
		if (HitResult.Actor.IsValid())
		{
			//UE_LOG(LogTemp, Warning, TEXT(" Hit Actor name is : %s"), *HitResult.GetActor()->GetName());

			AMainCharacter* Main = Cast<AMainCharacter>(HitResult.GetActor());
			if (Main)
			{
				if (bStrongAttack == true)
				{
					UGameplayStatics::ApplyDamage(Main, Damage + (Damage*0.5), AIController, this, DamageTypeClass);
					bStrongAttack = false;
					//UE_LOG(LogTemp, Warning, TEXT("Strong Attack . Damage is : %f"), Damage + (Damage * 0.5));
				}
				else
				{
					UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);
					//UE_LOG(LogTemp, Warning, TEXT("Damage is : %f"), Damage);
				}
				
				//UE_LOG(LogTemp, Warning, TEXT("Enemy::ApplyDamage"));
				//UE_LOG(LogTemp, Warning, TEXT("Damage : %f"), Damage);
			}
		}
		bWasHit = true;
	}
}

void AEnemy::AttackGiveDamage(ACharacter* Victim) //무기를 이용한 공격
{
	//UGameplayStatics::ApplyDamage(DamagedEnemy, PlayerDamage + WeaponDamage, GetController(), this, DamageTypeClass);
	AMainCharacter* Main = Cast<AMainCharacter>(Victim);
	if (Main)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Enemy::ApplyDamage"));
		if (bStrongAttack == true)
		{
			UGameplayStatics::ApplyDamage(Main, Damage + (Damage*0.5), AIController, this, DamageTypeClass);
			bStrongAttack = false;
			//UE_LOG(LogTemp, Warning, TEXT("Strong Attack . Damage is : %f"), Damage + (Damage * 0.5));
		}
		else
		{
			UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);
			//UE_LOG(LogTemp, Warning, TEXT("Damage is : %f"), Damage);
		}
		
	}
}

void AEnemy::DecrementalHealth(float TakeDamage)
{
	if (Health - TakeDamage <= 0.f)
	{
		Die();
	}
	else
	{
		Health -= TakeDamage;
	}

	if (EnemyWidget)
	{
		//UE_LOG(LogTemp, Warning, TEXT("EnemyWidget->EnemyHP is : %f"), Health);
		EnemyWidget->EnemyHP = Health; //Damage를 받을때 마다 update해준다.
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	//죽지 않았을때만 반응하도록.
	if(EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead)
	{
		//인식하지 못했는데 데미지를 받았으면 바로 돌게하기 위함
		AIController->UpdateTargetKey(DamageCauser);
		RotateToTarget();

		DecrementalHealth(DamageAmount);
		return DamageAmount;
	}
	return DamageAmount;
}

void AEnemy::Die()
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);
	AIController->UpdateTargetKey(nullptr);
	AIController->UpdateHasDetectedPlayer(false);

	Health = 0.f;
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (Anim)
	{
		Anim->Montage_Play(HitDeathMontage, 0.8f);
		Anim->Montage_JumpToSection(FName("Death"), HitDeathMontage);
		
	}
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::DeathEnd() //Die함수에서 재생하는 Animation의 Notify에서 호출.
{
	FTimerHandle DeathTimer;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DeathClear, 2.5f);
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AEnemy::DeathClear() //바로위 DeathEnd함수에서 타이머가 되면 호출됨.
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	SpawnLoot();
	if (LeftWeapon)
	{
		LeftWeapon->Destroy();
	}
	if (RightWeapon)
	{
		RightWeapon->Destroy();
	}
	Destroy();
}

void AEnemy::SpawnLoot()
{	

	int32 SoulQuantity = FMath::RandRange(SoulMin, SoulMax);

	UWorld* World = GetWorld();
	
	//LeftWeapon = GetWorld()->SpawnActor<AWeapon>(EnemyWeapon);

	/*UObject* SpawnSoul = Cast<UObject>(StaticLoadObject(UObject::StaticClass(), NULL, TEXT("/Game/Blueprints/Soul_BP.Soul_BP")));
	UBlueprint* SoulClass = Cast<UBlueprint>(SpawnSoul);
	FActorSpawnParameters SpawnParams;*/
	SpawnItemArea = FVector(40.f);
	/*static ConstructorHelpers::FObjectFinder<UBlueprint> SoulBP(TEXT("Blueprint'/Game/Blueprints/Soul_BP.Soul_BP'"));
	if (SoulBP.Object)
	{
		SoulClass = (UClass*)SoulBP.Object->GeneratedClass;
	}*/

	if (World && Soul)
	{
		for (int it = 0; it < SoulQuantity; it++)
		{
			const FVector SpawnLocation = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), SpawnItemArea);
			World->SpawnActor<ASoul>(Soul,SpawnLocation, FRotator(0.f));
		}
	}

	//if (World && SoulClass)
	//{
	//	for (int it =0; it < SoulQuantity; it++)
	//	{
	//		const FVector SpawnLocation = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), SpawnItemArea);
	//		World->SpawnActor<ASoul>(SoulClass->GeneratedClass, SpawnLocation, FRotator(0.f), SpawnParams);
	//	}
	//}
}


void AEnemy::Attack(UBlackboardComponent* BBComp)
{
	if (!bAttacking)// && EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead)
	{
		bAttacking = true;
		if (AnimInstance && AIController)
		{
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attack);

			MainPlayer = Cast<AMainCharacter>(BBComp->GetValueAsObject(AIController->TargetKey));

			checkf(MainPlayer, TEXT("Enemy::Attack() MainChar is NULL"));
			checkf(BBComp, TEXT("Enemy::Attack() BBComp is NULL"));

			if (MainPlayer->MovementStatus == EMovementStatus::EMS_Dead)
			{
				//공격때 Target(player)이 죽으면 TargetLost를 호출해서 Target을 nullptr로 변경해준다.
				AIController->TargetLost(MainPlayer);
			}

			//AIController에서 세팅하는 값.
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
					AnimInstance->Montage_JumpToSection(FName("Attack_1"), CloseCombatMontage); //Right Attack
					break;
				case 1:
					AnimInstance->Montage_JumpToSection(FName("Attack_2"), CloseCombatMontage); //Left Attack
					break;
				case 2:
					AnimInstance->Montage_JumpToSection(FName("Attack_Strong"), CloseCombatMontage); //Both side Attack
					bStrongAttack = true;
					break;
				default:
					break;
				}
			}
			if (!CloseCombat && DashAttack) //Dash공격이 true면
			{
				//FVector CurrentVector = GetActorLocation();
				//FVector DashVector(GetActorForwardVector() * 1200.f);
				//DashVector.Z = CurrentVector.Z; //오르막길, 내리막길에선 어떻게 하지??

				GetCharacterMovement()->MaxWalkSpeed = 950.f;

				if (AnimInstance && DashAttackCombatMontage)// && GetWorldTimerManager().TimerExists(DashAttackHandle))
				{
					AnimInstance->Montage_Play(DashAttackCombatMontage, 1.2f);

					if (AnimInstance->Montage_IsPlaying(DashAttackCombatMontage))
					{
						GetWorld()->GetTimerManager().SetTimer(DashAttackHandle, [this]
							{
								if (EnemyMovementStatus == EEnemyMovementStatus::EMS_Dead)
								{
									GetWorldTimerManager().ClearTimer(DashAttackHandle);
									return;
								}

								/*https://www.notion.so/bigdumpling/b25aa96658004f3dbfdeed7624452ecc */
								FName PlayingSection = AnimInstance->Montage_GetCurrentSection(DashAttackCombatMontage); 
								//UE_LOG(LogTemp, Warning, TEXT("Dash anim cursection is : %s"), *PlayingSection.ToString());
								if (PlayingSection != TEXT("Dash"))
								{
									return;
								}
								else
								{
									FAIMoveRequest MoveReq;
									MoveReq.SetGoalActor(MainPlayer);
									MoveReq.SetAcceptanceRadius(1.f);
									AIController->MoveTo(MoveReq);
								}
							}, GetWorld()->GetDeltaSeconds(), true);
					}
				}
			}
		}
	}
}

void AEnemy::AttackEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("Delegate:: Enemy:: Attack End Function call"));
	bAttacking = false;
	bWasHit = false; //ReturnHit()에서 사용함.
	GetWorldTimerManager().ClearTimer(DashAttackHandle);
}


void AEnemy::RotateToTarget()
{
	float InterpSpeed = 80.0f;
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


void AEnemy::ActivateCollision()
{
	UAnimMontage* CurrentPlay = AnimInstance->GetCurrentActiveMontage();
	FName SectionName = AnimInstance->Montage_GetCurrentSection(CurrentPlay);

	if (SectionName == FName("Attack_1") && RightWeapon) //Right Attack
	{
		UE_LOG(LogTemp, Warning, TEXT("Right Attack -> Activate Collision"));
		RightWeapon->ActivateCollision();
	}
	if (SectionName == FName("Attack_2") && LeftWeapon) //Left Attack
	{
		UE_LOG(LogTemp, Warning, TEXT("Left Attack -> Activate Collision"));
		LeftWeapon->ActivateCollision();
	}
	if (SectionName == FName("Attack_Strong") && LeftWeapon && RightWeapon) //Both side Attack
	{
		UE_LOG(LogTemp, Warning, TEXT("Both side -> Activate Collision"));
		RightWeapon->ActivateCollision();
		LeftWeapon->ActivateCollision();
	}
}

void AEnemy::DeactivateCollision()
{
	if (LeftWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Left -> Deactivate Collision"));
		LeftWeapon->DeactivateCollision();
	}
	if (RightWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Right -> Deactivate Collision"));
		RightWeapon->DeactivateCollision();
	}
}