// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"


// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
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
	bShiftKeyDown = false;
	bEKeyDown = false;
	bShiftKeyDown = false;
	bJumpKeyDown = false;

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

	

	/*******************************/
	//***** Character Stats *****//
	/*******************************/
	MaxHealth = 100.f;
	Health = 100.f;
	MaxStamina = 300.f;
	Stamina = 300.f;

	Coins = 0;
	Souls = 0;
	
	/*******************************/
	//***** Player Combat  *****//
	/*******************************/
	bAttacking = false;
	bSaveAttack = false;
	AttackCount = 0;
	CurHeight = 0.f;

}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float DeltaStaminaDrain = StaminaDrainRate * DeltaTime;
	float DeltaStaminaRecovery = StaminaRecoveryRate * DeltaTime;
	

	/***Line Trace (캐릭터 밑에서부터 Mesh까지의 거리측정 ****/
	FHitResult OutHit;
	FVector StartPoint = GetCharacterMovement()->GetActorFeetLocation();
	FVector EndPoint = FVector(StartPoint.X, StartPoint.Y, -500.f);

	FCollisionQueryParams CollisionParams;
	DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 1.f, 0, 2);

	bool isHit = GetWorld()->LineTraceSingleByChannel(OutHit, StartPoint, EndPoint, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (isHit)
	{
		if (OutHit.bBlockingHit)
		{
			CurHeight = StartPoint.Z - OutHit.ImpactPoint.Z;
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("SPoint xyz : %.2f %.2f %.2f"), 
					StartPoint.X, StartPoint.Y, StartPoint.Z));
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Hit Imact Point : %s"), 
					*OutHit.ImpactPoint.ToString()));
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Calc Height : %f"),
					CurHeight));
			}
		}
	}

	//StaminaStatus 관리.
	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown)
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
		else //Shift Key가 눌리지 않으면
		{
			if (Stamina + DeltaStaminaRecovery <= MaxStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Recovery);
			}
		}
		break;

	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown)
		{
			if(Stamina - DeltaStaminaDrain <= 0.f)
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
		if (bShiftKeyDown)
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
		if (bShiftKeyDown) //눌렸을때는 바로 넘겨준다.
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

void AMainCharacter::Jump()
{
	if (!bAttacking)
	{
		Super::Jump();
		bJumpKeyDown = true;
	}
}

void AMainCharacter::StopJumping()
{
	Super::StopJumping();
	bJumpKeyDown = false;
}

/*************** Movement **************/
//////////   Movement 관련 함수  ////////
/**************************************/

void AMainCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.f) && (!bAttacking))
	{
		//Cameraboom도 ControlRotation을 이용. ControlRotation을 이용해서 회전방향으로 진행.
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		//이 YawRotation값으로 회전행렬을 만들고 이로부터 X축을 얻는다. (회전체의 기준방향으로 Forward벡터(x)를 얻음)
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.f) && (!bAttacking))
	{
		const FRotator Rotation = Controller->GetControlRotation();
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

/************ Money **************/
///////// Coin, Soul 관련 함수 /////
/*********************************/
void AMainCharacter::IncrementCoin(int32 Amount)
{
	Coins += Amount;
}


/*************** Damage ****************/
//////////   Damage 관련 함수   /////////
/***************************************/
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

}

/************ Weapon **************/
///////// Weapon 관련 함수 /////
/*********************************/
void AMainCharacter::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	if (EquippedWeapon) //기존 장착된게 있으면
	{
		EquippedWeapon->Destroy(); //destory하고 장착.
	}
	EquippedWeapon = WeaponToSet;
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

	if (AnimInstance && CombatMontage)
	{
		bAttacking = true;
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
	AttackCount = 0;
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