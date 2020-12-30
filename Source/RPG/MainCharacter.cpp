// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Weapon.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "Explosive.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense.h"
#include "Components/TimelineComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"


// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	////////////TEST AI/////////////
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));


	bCapsuleHit = false;

	/*******************************/
	//------   카메라  관련   ------//
	/*******************************/
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; //CameraBoom에 의존하기 위함.
	
	/////카메라 회전 /////
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//회전시 카메라에만 영향 //
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	/*******************************/
	//-- Player Input ---//
	/*******************************/
	bLMBDown = false;
	bRMBDown = false;
	bShiftKeyDown = false;
	bEKeyDown = false;
	bShiftKeyDown = false;
	bJumpKeyDown = false;
	bMoveForward = false;
	bMoveRight = false;
	bFkeyDown = false;

	/*******************************/
	//-- Player Movement  ---//
	/*******************************/

	////움직임 (점프) 수정 ////
	GetCharacterMovement()->bOrientRotationToMovement = true; //움직인 방향 = 진행방향으로 설정
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f); //위의 회전속도값.
	GetCharacterMovement()->JumpZVelocity = 550.f; //점프 높이 설정.
	GetCharacterMovement()->AirControl = 0.3f;

	
	//Enum 초기화
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;
	
	//----Movement ----//
	RunningSpeed = 600.f;
	SprintingSpeed = 900.f;

	StaminaRecoveryRate = 90.f;
	StaminaDrainRate = 70.f;
	MinStamina = 75.f;

	//낙하 데미지 관련//
	FallingDamage = 0.f;
	FallingMaxHeight = 0.f;
	//bFell = false;

	/*******************************/
	//***** Character Stats *****//
	/*******************************/
	MaxHealth = 100.f;
	Health = 100.f;
	MaxStamina = 300.f;
	Stamina = 300.f;
	
	PlayerDamage = 20.f;

	Coins = 0;
	Souls = 0;
	
	/*******************************/
	//***** Player Combat  *****//
	/*******************************/
	bAttacking = false;
	bSaveAttack = false;
	bAbilitySmash = false;
	AttackCount = 0;
	CurHeight = 0.f;

}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	

	PlayerController = Cast<AMainPlayerController>(GetController());

	////////////TEST AI/////////////
	StimuliSourceComponent->bAutoRegister = true;
	StimuliSourceComponent->RegisterForSense(SenseSight); //Sight Sense를 등록.

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AMainCharacter::CapsuleOnHit);
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float DeltaStaminaDrain = StaminaDrainRate * DeltaTime;
	float DeltaStaminaRecovery = StaminaRecoveryRate * DeltaTime;
	
	//플레이어 위치 TEST용.
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Cur Char Lo : %s"),*GetActorLocation().ToString()));
	}*/

	//캐릭터의 높이 관련
	{
		/***Line Trace (캐릭터 밑에서부터 Mesh까지의 거리측정 ****/
		FHitResult OutHit;
		FVector StartPoint = GetCharacterMovement()->GetActorFeetLocation();
		FVector EndPoint = FVector(StartPoint.X, StartPoint.Y, -800.f);

		FCollisionQueryParams CollisionParams;
		//DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 1.f, 0, 2); //Line Trace 시각화 Debug

		bool isHit = GetWorld()->LineTraceSingleByChannel(OutHit, StartPoint, EndPoint, ECollisionChannel::ECC_Visibility, CollisionParams);

		if (isHit)
		{
			if (OutHit.bBlockingHit)
			{
				CurHeight = StartPoint.Z - OutHit.ImpactPoint.Z;
				//if (GEngine) //뷰포트 출력
				//{
				//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Start Point : %s"),
				//		*StartPoint.ToString()));
				//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Hit Imact Point : %s"),
				//		*OutHit.ImpactPoint.ToString()));
				//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Calc Height : %.2f"),
				//		CurHeight));
				//}
			}
		}

		//낙하 데미지
		float AfterHeight = 0.f;
		if (GetCharacterMovement()->IsFalling()) //공중에서 떨어질때 Damage를 계산 및 적용.
		{
			AfterHeight = 0.f;
			FallingDamageCalc(); //낙하시 최대 높이를 구하는함수.
		}
		else if (GetCharacterMovement()->IsFalling() == false)
		{
			AfterHeight = OutHit.ImpactPoint.Z; //착지한 현재 높이를 넘겨준다.
			TakeFallingDamage(AfterHeight);
		}
	}
	//StaminaStatus 관리.
	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		//움직임+ShiftKey를 눌렀을때만 Sprint, Stamina가 drain되도록 구현함. 모든 Status에 적용중.
		if (bShiftKeyDown && (bMoveForward || bMoveRight)) 
		{
			//if (bMoveForward || bMoveRight)
			{
				if (Stamina - DeltaStaminaDrain <= MinStamina)
				{
					SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				}
				else
				{
					Stamina -= DeltaStaminaDrain;
				}
				SetMovementStatus(EMovementStatus::EMS_Sprint);
			}
		}
		else //Shift Key가 눌리지 않으면
		{
			if (Stamina + DeltaStaminaRecovery <= MaxStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Recovery);
			}
		}
		break;

	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown && (bMoveForward || bMoveRight))
		{
			if (Stamina - DeltaStaminaDrain <= 0.f)
			{
				Stamina = 0.f;
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else
			{
				Stamina -= DeltaStaminaDrain;
				SetMovementStatus(EMovementStatus::EMS_Sprint);
			}
		}
		else //Shitkey를 안눌렀을때
		{
			SetStaminaStatus(EStaminaStatus::ESS_Recovery);
		}
		break;

	case EStaminaStatus::ESS_Exhausted:
		if (bShiftKeyDown && (bMoveForward || bMoveRight))
		{
			Stamina = 0.f;
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		else //Shift 키를 안눌렀을때
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovery);
		}

		break;

	case EStaminaStatus::ESS_ExhaustedRecovery:
		if (Stamina + DeltaStaminaRecovery >= MinStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Recovery);
		}
		else
		{
			Stamina += DeltaStaminaRecovery;
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		
		break;

	case EStaminaStatus::ESS_Recovery:
		if (bShiftKeyDown && (bMoveForward || bMoveRight)) //눌렸을때는 바로 넘겨준다.
		{
			if (Stamina >= MinStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
			}
			else
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
			}	
		}
		else
		{
			if (Stamina + DeltaStaminaRecovery >= MaxStamina)
			{
				Stamina = MaxStamina;
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
			}
			else
			{
				Stamina += DeltaStaminaRecovery;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	default:
		break;
	}

}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//valid체크 매크로
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMainCharacter::LookUpRate);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AMainCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AMainCharacter::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AMainCharacter::ShiftKeyUp);
	PlayerInputComponent->BindAction("InteractionKey", EInputEvent::IE_Pressed, this, &AMainCharacter::EKeyDown);
	PlayerInputComponent->BindAction("InteractionKey", EInputEvent::IE_Released, this, &AMainCharacter::EKeyUp);
	PlayerInputComponent->BindAction("LMB", EInputEvent::IE_Pressed, this, &AMainCharacter::LMBDown);
	PlayerInputComponent->BindAction("LMB", EInputEvent::IE_Released, this, &AMainCharacter::LMBUp);

	PlayerInputComponent->BindAction("RMB", EInputEvent::IE_Pressed, this, &AMainCharacter::RMBDown);
	PlayerInputComponent->BindAction("RMB", EInputEvent::IE_Released, this, &AMainCharacter::RMBUp);
	PlayerInputComponent->BindAction("Ability_F", EInputEvent::IE_Pressed, this, &AMainCharacter::FKeyDown);
	PlayerInputComponent->BindAction("Ability_F", EInputEvent::IE_Released, this, &AMainCharacter::FKeyUp);

}

/*************** Input **************/
//////////   Input 관련 함수  ////////
/**************************************/
void AMainCharacter::ShiftKeyDown()
{
	bShiftKeyDown = true;
}
void AMainCharacter::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMainCharacter::EKeyDown()
{
	bEKeyDown = true;
	if (OverlappingItem)
	{
		AWeapon* Weapon = Cast<AWeapon>(OverlappingItem);
		if(Weapon)
		{ 
			Weapon->Equip(this);
		}
	}
}
void AMainCharacter::EKeyUp()
{
	bEKeyDown = false;
}

void AMainCharacter::LMBDown()
{
	bLMBDown = true;

	if (EquippedWeapon)
	{
		if (!bAttacking)
		{
			Attack();	
		}
		else
		{
			bSaveAttack = true;
		}
	}
}

void AMainCharacter::LMBUp()
{
	bLMBDown = false;
}

void AMainCharacter::RMBDown()
{
	bRMBDown = true;
	Ability_ThrowWeapon();
}

void AMainCharacter::RMBUp()
{
	bRMBDown = false;
}

void AMainCharacter::Jump()
{
	if (!bAttacking)
	{
		Super::Jump();
		ACharacter::Jump();
		bJumpKeyDown = true;
	}
}

void AMainCharacter::StopJumping()
{
	Super::StopJumping();
	ACharacter::StopJumping();
	bJumpKeyDown = false;
}

void AMainCharacter::FKeyDown()
{
	bFkeyDown = true;
	Ability_Smash();
}

void AMainCharacter::FKeyUp()
{
	bFkeyDown = false;
}

//CapsuleComponent 충돌 관련
void AMainCharacter::CapsuleOnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Hit.bBlockingHit && bAttacking)
	{
		bCapsuleHit = true;
		UE_LOG(LogTemp, Warning, TEXT("Capsule hit! %s"), *(Hit.Actor->GetFName().ToString()));
	}
}


/*************** Movement **************/
//////////   Movement 관련 함수  ////////
/**************************************/

void AMainCharacter::MoveForward(float Value)
{
	bMoveForward = false;
	if ((PlayerController != nullptr) && (Value != 0.f) && (!bAttacking))
	{
		bMoveForward = true;
		//Cameraboom도 ControlRotation을 이용. ControlRotation을 이용해서 회전방향으로 진행.
		const FRotator Rotation = PlayerController->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		//이 YawRotation값으로 회전행렬을 만들고 이로부터 X축을 얻는다. (회전체의 기준방향으로 Forward벡터(x)를 얻음)
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::MoveRight(float Value)
{
	bMoveRight = false;
	if ((PlayerController != nullptr) && (Value != 0.f) && (!bAttacking))
	{
		bMoveRight = true;
		const FRotator Rotation = PlayerController->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::TurnAtRate(float Rate)
{
	//매 frame마다 BaseRate만큼 회전한다.
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}
void AMainCharacter::LookUpRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//Movement Status
void AMainCharacter::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprint)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}


void AMainCharacter::CharacterRotate() //공격때 사용함.
{
	float ForwardAxis = GetInputAxisValue(FName("MoveForward"));
	float RightAxis = GetInputAxisValue(FName("MoveRight"));
	FVector Direction = FVector(ForwardAxis, RightAxis, 0.f).GetSafeNormal(); //입력에 대한 방향.

	
	FRotator Rotation = FRotator(0.f, PlayerController->GetControlRotation().Yaw + Direction.Rotation().Yaw, 0.f); //회전방향 절대축에 입력방향을 더함

	SetActorRotation(Rotation);
}

/************ Money **************/
///////// Pickup 관련 함수 /////
/*********************************/
void AMainCharacter::IncrementCoin(int32 Amount)
{
	Coins += Amount;
}

void AMainCharacter::IncrementSoul(int32 Amount)
{
	Souls += Amount;
}

void AMainCharacter::IncrementHealth(float Amount)
{
	Health += Amount;
	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
}

/*************** Damage ****************/
//////////   Damage 관련 함수   /////////
/***************************************/

//무기를 이용한 공격, Weapon Class에서 호출함.
void AMainCharacter::AttackGiveDamage(AEnemy* DamagedEnemy, float WeaponDamage) //공격후 Damage를 줌.
{
	
	UGameplayStatics::ApplyDamage(DamagedEnemy, PlayerDamage + WeaponDamage, GetController(), this, DamageTypeClass);

	//UE_LOG(LogTemp, Warning, TEXT("MainPlayer->AttackDamage()"));
	//UE_LOG(LogTemp, Warning, TEXT("Player Base Damage is : %f, EquippedWeapon Damage is : %f"), PlayerDamage, WeaponDamage);
	//UE_LOG(LogTemp, Warning, TEXT("Total Damage is : %f"), PlayerDamage + WeaponDamage);
}


//범위공격, AnimNotifyState_RangeAttack에서 호출함( 애님 노티파이 스테이트)
void AMainCharacter::AttackRangeDamage() //Player의 범위 공격 (스킬 같은것.)
{
	//UE_LOG(LogTemp, Warning, TEXT("MainCharacter::AttackRangeDamage()"));

	//Damage부터 확인.
	float Damage = PlayerDamage;
	if (GetCharacterMovement()->IsFalling()) //떨어지고 있으면, 낙하높이에 15%의 데미지를 더 줌. 
	{
		Damage += CurHeight * 0.15f;
		//UE_LOG(LogTemp, Warning, TEXT("Damage is : %f, Weapon Damage is : %f, TotalDamage is : %f"), Damage, EquippedWeapon->WeaponDamage, Damage + EquippedWeapon->WeaponDamage);
	}

	TArray<FHitResult>OutHit;
	FVector StartLocation = GetActorLocation(); //StartLocation
	FVector EndLocation;
	ECollisionChannel CollisionChannel;
	FCollisionShape CollisionShape;
	FCollisionQueryParams Params(TEXT("PlayerRangeDamage"), false, this);
	FCollisionResponseParams ResponseParams;

	//디버깅용 변수
	FVector WeaponLength = EquippedWeapon->CombatCollision->GetScaledBoxExtent() * 2.0f;
	float ZSize = WeaponLength.Z;
	float YSize = WeaponLength.Y;
	float XSize = WeaponLength.X;


	//스킬 사용시 EndLocation, Collision을 다르게 지정해준다.
	if (bAbilitySmash)
	{
		
		EndLocation = GetActorLocation();
		CollisionChannel = ECollisionChannel::ECC_WorldDynamic; //WorldDynamic 요소를 맞춤(전부다맞춤)
		CollisionShape = FCollisionShape::MakeSphere(400.f);

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AllDynamicObjects;

		GetWorld()->SweepMultiByObjectType(OutHit, StartLocation, EndLocation, FQuat::Identity, ObjectParams,
			CollisionShape, Params);
	}
	else
	{
		EndLocation = GetActorForwardVector() * WeaponLength.Z + StartLocation;
		CollisionChannel = ECollisionChannel::ECC_Pawn;
		CollisionShape = FCollisionShape::MakeCapsule(WeaponLength);
		ResponseParams = FCollisionResponseParams::DefaultResponseParam;

		//Weapon의 CombatCollision * 2 크기를 가진 캡슐모양으로 판정한다.	
		GetWorld()->SweepMultiByChannel(OutHit, StartLocation, EndLocation, FQuat::Identity,
			CollisionChannel, CollisionShape, Params, ResponseParams);
	}

	//디버깅용
	{
		if (bAbilitySmash)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), 400.f, int32(12), FColor::Red, false, 2.f, (uint8)nullptr, 1.0f);
		}
		else
		{
			DrawDebugCapsule(GetWorld(), EndLocation,
				ZSize * 0.5 + XSize + YSize, XSize + YSize, FRotationMatrix::MakeFromZ(GetActorForwardVector() * ZSize).ToQuat(),
				FColor::Red, false, 2.0f);
		}
	}
	

	if (OutHit.Num() == 0) return;

	for (auto Hit : OutHit)
	{
		AEnemy* Enemy = Cast<AEnemy>(Hit.GetActor());
		AExplosive* Explosive = Cast<AExplosive>(Hit.GetActor());
		if (bAbilitySmash)
		{
			if (Enemy)
			{
				UE_LOG(LogTemp, Warning, TEXT("victim name is %s"), *(Hit.Actor->GetFName().ToString()));
				UE_LOG(LogTemp, Warning, TEXT("RangeAttack AbilitySmash Apply Damage"));
				TArray<AActor*>IgnoreActor;

				UGameplayStatics::ApplyRadialDamage(this, (Damage * 1.5) + EquippedWeapon->WeaponDamage, GetActorLocation(), 600.f, DamageTypeClass, IgnoreActor, this, GetController(), true,
					ECollisionChannel::ECC_WorldStatic); //마지막 인자값은, 타격 원점과 액터사이에 해당 채널을 막는 액터가 있다면 damamge를 받지 않는다는것임.

				//디버깅용
				DrawDebugSphere(GetWorld(), GetActorLocation(), 400.f, int32(12), FColor::Green, false, 2.f, (uint8)nullptr, 1.0f);
			}
			if (Explosive)
			{
				Explosive->Delete();
			}
		}
		else
		{
			if (Enemy)
			{
				UE_LOG(LogTemp, Warning, TEXT("RangeAttack Apply Damage"));
				UGameplayStatics::ApplyDamage(Enemy, Damage + EquippedWeapon->WeaponDamage, GetController(), this, DamageTypeClass);
				//디버깅용
				{
					DrawDebugCapsule(GetWorld(), EndLocation,
						ZSize * 0.5 + XSize + YSize, XSize + YSize, FRotationMatrix::MakeFromZ(GetActorForwardVector() * ZSize).ToQuat(),
						FColor::Green, false, 2.0f);
					//UE_LOG(LogTemp, Warning, TEXT("Player Range Attack success, Damage is : %f, Weapon Damage is : %f, TotalDamage is : %f"), Damage, EquippedWeapon->WeaponDamage, Damage + EquippedWeapon->WeaponDamage);
				}
			}
		}
	}
}

void AMainCharacter::FallingDamageCalc() //낙하시 최대 높이를 구한다.
{
	if (FallingMaxHeight < CurHeight)
	{
		FallingMaxHeight = CurHeight;
	}
}

void AMainCharacter::TakeFallingDamage(float AfterHeight)
{
	//떨어지고 착지 이후에 데미지 계산, 적용 및 초기화를 해준다.
	
	if (bAbilitySmash == true) //해당 ability가 재생중(실행중)이면 낙하뎀지 무시.
	{
		FallingMaxHeight = 0.f;
		return;
	}
	//떨어질때의 최대높이와 착지후 현재높이의 차이가 자기키의 1.5배가 넘으면 데미지를 받게 했다. -> 높은곳에서 높은곳으로 점프할때 데미지 받는걸 방지하기 위해서.
	if (FallingMaxHeight-AfterHeight >= GetDefaultHalfHeight() * 3.f) //자기키의  1.5배가 되면 낙하데미지를 받는다. 
	{
		FallingDamage = FallingMaxHeight * 0.05f; //높이에서 5%를 데미지로 준다.
		DecrementHealth(FallingDamage);
		//디버깅용
		/*UE_LOG(LogTemp, Warning, TEXT("FMH is : %f, AfterHeight is : %f"), FallingMaxHeight, AfterHeight);
		UE_LOG(LogTemp, Warning, TEXT("Subtract is : %f"), FallingMaxHeight - AfterHeight);
		UE_LOG(LogTemp, Warning, TEXT("Falling Damage is : %f"), FallingDamage);*/
	}
	//관련 변수 초기화.
	FallingDamage = 0.f;
	FallingMaxHeight = 0.f;

}

float AMainCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	//UE_LOG(LogTemp, Warning, TEXT("MainCharacter::TakeDamage()::::Damage Causer : %s , Damage : %f"), *DamageCauser->GetName(), DamageAmount);
	
	if (bAbilitySmash == false) //해당 어빌리티 사용시 Damage를 받지 않기 하기 위함.
	{
		DecrementHealth(DamageAmount);
	}
	return DamageAmount;
}


void AMainCharacter::DecrementHealth(float Amount)
{
	if (Health - Amount <= 0.f)
	{
		Die();
	}
	else Health -= Amount;
}
void AMainCharacter::Die()
{
	Health = 0;
	UE_LOG(LogTemp, Warning, TEXT("AMainCharacter::Die()"));
}

/************ Weapon **************/
///////// Weapon 관련 함수 /////
/*********************************/
void AMainCharacter::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	if (EquippedWeapon) //기존 장착된게 있으면
	{
		EquippedWeapon->Destroy(); //destory하고 장착, 나중에 Inventory에 넣어주자.
	}
	EquippedWeapon = WeaponToSet;
	EquippedWeapon->SetWeaponOwner(this);
}

/************ Combat **************/
///////// Combat 관련 함수 /////
/*********************************/

void AMainCharacter::Attack()
{
	bool fall = GetCharacterMovement()->IsFalling();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if (fall && CalcAirAttack()) //공중에 있으면 AttackAir함수로
	{
		AttackAir();
		return;
	}
	else if (MovementStatus == EMovementStatus::EMS_Sprint) //스프린트 상태일때 대쉬공격
	{
		SptirntAttack();
		return;
	}

	//공중공격, 대쉬공격을 우선순위로 하고 난뒤/ 일반 공격.

	if (AnimInstance && CombatMontage)
	{
		bAttacking = true;
		
		CharacterRotate(); //공격중 회전입력이 들어오면 회전하도록 해줌.

		if(!bSaveAttack && bAttacking)
		{
			AnimInstance->Montage_Play(CombatMontage, 1.0f);
			switch (AttackCount)
			{
			case 0:
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
				AttackCount++;
				break;
			case 1:
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
				AttackCount++;
				break;
			case 2:
				AnimInstance->Montage_JumpToSection(FName("Attack_3"), CombatMontage);
				AttackCount++;
				break;
			case 3:
				AnimInstance->Montage_JumpToSection(FName("Attack_4"), CombatMontage);
				AttackCount = 0;
				break;
			default:
				break;
			}
		}
	}
}

bool AMainCharacter::CalcAirAttack()
{
	bool fall = GetCharacterMovement()->IsFalling();
	float HalfHeight = GetDefaultHalfHeight();

	if (CurHeight >= (HalfHeight/2) && fall)
	{
		return true;
	}
	else return false;
}

void AMainCharacter::AttackAir()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	float HalfHeight = GetDefaultHalfHeight();
	
	if (AnimInstance && AirCombatMontage && !bAttacking)
	{
		bAttacking = true;
		AnimInstance->Montage_Play(AirCombatMontage, 1.2f);
		if (CurHeight >= HalfHeight*2)
		{
			AnimInstance->Montage_JumpToSection(FName("AirAttack_2"), AirCombatMontage);
			UE_LOG(LogTemp, Warning, TEXT("Land Crash Attack // CurHeight : %f"), CurHeight);
		}
		else
		{
			AnimInstance->Montage_JumpToSection(FName("AirAttack_1"), AirCombatMontage);
			UE_LOG(LogTemp, Warning, TEXT("Air Attack // CurHeight : %f"), CurHeight);
		}
	}
}

void AMainCharacter::SptirntAttack()
{
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (Anim && SprintCombatMontage)
	{
		FVector OriginLocation = GetActorLocation();
		FVector TargetVector = GetActorForwardVector() * 725.f; //GetVelocity().Size();
		//float HalfHeight = GetDefaultHalfHeight();
		TargetVector.Z = 0.f;
		bAttacking = true;
		Anim->Montage_Play(SprintCombatMontage, 1.3f);
		//GetCapsuleComponent()->SetCapsuleHalfHeight(HalfHeight / 2);

		GetWorldTimerManager().SetTimer(SprintAttackTimer, [=]
			{
				FVector CurrentLocation = GetActorLocation();
				bool bReachPoint = FVector().PointsAreNear(CurrentLocation, OriginLocation + TargetVector, 5.0f);
				
				//디버깅
				/*
				DrawDebugSphere(GetWorld(), OriginLocation + TargetVector, 25.f, 6, FColor::Green, false, 4.f, (uint8)nullptr, 1.0f);
				UE_LOG(LogTemp, Warning, TEXT("-------------------------Timer ------------------------"));
				UE_LOG(LogTemp, Warning, TEXT("Origin Location : %s -> ForwardVector : %s :::: TargetVector : %s"), *OriginLocation.ToString(), *GetActorForwardVector().ToString() ,*TargetVector.ToString());
				*/

				if (bReachPoint)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Move request success And Clear Timer"));
					
					return;
				}
				else
				{
					//UE_LOG(LogTemp, Warning, TEXT("Cur Location : %s -> Target Location : %s"), *CurrentLocation.ToString(), *(OriginLocation + TargetVector).ToString());
					AddMovementInput(TargetVector, 1.0f );
				}

			}, GetWorld()->GetDeltaSeconds(), true);
	}
}

void AMainCharacter::ComboSave()
{
	if (bSaveAttack)
	{
		bSaveAttack = false;
		Attack();
	}
}

void AMainCharacter::ComboReset()
{
	bAttacking = false;
	bSaveAttack = false;
	bCapsuleHit = false; //smash ability끝나고 capsule hit을 풀어주기 위함.
	bAbilitySmash = false;
	SetCanBeDamaged(true); //smash ability끝나고 무적을 풀어주기위함.

	AttackCount = 0;
	if (GetWorldTimerManager().IsTimerActive(SprintAttackTimer))
	{
		//GetCapsuleComponent()->SetCapsuleHalfHeight(95.f);
		GetWorldTimerManager().ClearTimer(SprintAttackTimer);
	}
}

void AMainCharacter::Ability_ThrowWeapon()
{
	if (bRMBDown)
	{
		if (EquippedWeapon && !bAttacking)
		{
			FName AttachedSocketName = EquippedWeapon->GetAttachParentSocketName();
			//UE_LOG(LogTemp, Warning, TEXT("Weapon Attach at %s"), *(AttachedSocketName.ToString()));
			if (AttachedSocketName != NAME_None)
			{
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if (AnimInstance && AbilityThrowWeaponMontage)
				{
					AnimInstance->Montage_Play(AbilityThrowWeaponMontage, 2.25f);
					AnimInstance->Montage_JumpToSection(FName("Execute"), AbilityThrowWeaponMontage);

					EquippedWeapon->ThrowWeapon(this, AttachedSocketName);
					//EquippedWeapon->SetWeaponOwner(nullptr);
					EquippedWeapon = nullptr;
					//FVector WeaponThrow = FMath::VInterpTo(EquippedWeapon->GetActorLocation(), Destination, GetWorld()->GetDeltaSeconds(), 10.f);
				}



			}
		}
	}
}

void AMainCharacter::Ability_ThrowWeapon_Finish() //AWeapon::ReceiveWeapon에서 호출
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AbilityThrowWeaponMontage)
	{
		AnimInstance->Montage_Play(AbilityThrowWeaponMontage, 1.5f);
		AnimInstance->Montage_JumpToSection(FName("Finish"), AbilityThrowWeaponMontage);
	}
}

void AMainCharacter::Ability_Smash()
{
	if (EquippedWeapon && AbilitySmashMontage && !bAttacking && !GetCharacterMovement()->IsFalling())
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			bAttacking = true;
			bAbilitySmash = true;

			FVector OriginLocation = GetActorLocation();
			FVector TargetLocation = GetActorForwardVector() * 400.f;
			TargetLocation.Z = 320.f; //타겟목적지의 높이를 설정해줌.

			FVector LastLocation = OriginLocation + TargetLocation;

			AnimInstance->Montage_Play(AbilitySmashMontage, 1.3f);
			AnimInstance->Montage_JumpToSection(FName("Execute"), AbilitySmashMontage);

			SetCanBeDamaged(false); //해당 스킬 사용중 무적상태를 부여한다.
		
			GetWorldTimerManager().SetTimer(AbilitySmashHandle, [=]{

				bool bIsNear = FVector::PointsAreNear(GetActorLocation(), LastLocation, 80.f); //near판정
				bool bAnimPlay = AnimInstance->Montage_IsActive(AbilitySmashMontage); //anim 재생 판정

				GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); //해당 스킬 사용시 pawn을 무시한다 ->무적이동상태

				//디버깅용
				//DrawDebugSphere(GetWorld(), LastLocation, 25.f, 6, FColor::Green, false, 4.f, (uint8)nullptr, 1.0f);

				if (bIsNear || bAnimPlay == false || bCapsuleHit == true) //bAnimplay는 혹시모를 버그때문에 넣어뒀다.
				{
					Ability_Smash_Finish();
				}
				else
				{
					GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying); //이렇게 해주니까 진짜 부드럽게 움직인다. -> 뛰려면 이렇게 해둬야함.
					AddMovementInput(TargetLocation, 1.0f, true);
				}
				}, GetWorld()->GetDeltaSeconds(), true);
		}
	}
}

void AMainCharacter::Ability_Smash_Finish() //공중에 도착한뒤 풀어주는것.
{
	//bCapsuleHit = false;
	GetWorldTimerManager().ClearTimer(AbilitySmashHandle);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
}

/*******************************/
//--- Game Level,Save,Load  ---//
/*******************************/

//나중에 MainChar의 Stats과 weapon들을 넘겨줘야함.
void AMainCharacter::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString MapName = World->GetMapName();

		FName CurrentLevelName(*MapName);

		//변경할 Level과 현재Level이 다를때 Open world를 한다.
		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}