// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	////////////TEST AI/////////////
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));

	/*******************************/
	//------   ī�޶�  ����   ------//
	/*******************************/
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; //CameraBoom�� �����ϱ� ����.
	
	/////ī�޶� ȸ�� /////
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//ȸ���� ī�޶󿡸� ���� //
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
	bMoveForward = false;
	bMoveRight = false;

	/*******************************/
	//-- Player Movement  ---//
	/*******************************/

	////������ (����) ���� ////
	GetCharacterMovement()->bOrientRotationToMovement = true; //������ ���� = ����������� ����
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f); //���� ȸ���ӵ���.
	GetCharacterMovement()->JumpZVelocity = 550.f; //���� ���� ����.
	GetCharacterMovement()->AirControl = 0.3f;

	
	//Enum �ʱ�ȭ
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
	
	////////////TEST AI/////////////
	StimuliSourceComponent->bAutoRegister = true;
	StimuliSourceComponent->RegisterForSense(SenseSight); //Sight Sense�� ���.
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float DeltaStaminaDrain = StaminaDrainRate * DeltaTime;
	float DeltaStaminaRecovery = StaminaRecoveryRate * DeltaTime;
	
	//�÷��̾� ��ġ TEST��.
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Cur Char Lo : %s"),*GetActorLocation().ToString()));
	}*/

	/***Line Trace (ĳ���� �ؿ������� Mesh������ �Ÿ����� ****/
	{
		FHitResult OutHit;
		FVector StartPoint = GetCharacterMovement()->GetActorFeetLocation();
		FVector EndPoint = FVector(StartPoint.X, StartPoint.Y, -800.f);
		
		FCollisionQueryParams CollisionParams;
		//DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 1.f, 0, 2); //Line Trace �ð�ȭ Debug

		bool isHit = GetWorld()->LineTraceSingleByChannel(OutHit, StartPoint, EndPoint, ECollisionChannel::ECC_Visibility, CollisionParams);

		if (isHit)
		{
			if (OutHit.bBlockingHit)
			{
				CurHeight = StartPoint.Z - OutHit.ImpactPoint.Z;
				//if (GEngine) //����Ʈ ���
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
	}
	//StaminaStatus ����.
	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		//������+ShiftKey�� ���������� Sprint, Stamina�� drain�ǵ��� ������. ��� Status�� ������.
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
		else //Shift Key�� ������ ������
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
		else //Shitkey�� �ȴ�������
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
		else //Shift Ű�� �ȴ�������
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
		if (bShiftKeyDown && (bMoveForward || bMoveRight)) //���������� �ٷ� �Ѱ��ش�.
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

	//validüũ ��ũ��
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
//////////   Input ���� �Լ�  ////////
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

/*************** Movement **************/
//////////   Movement ���� �Լ�  ////////
/**************************************/

void AMainCharacter::MoveForward(float Value)
{
	bMoveForward = false;
	if ((Controller != nullptr) && (Value != 0.f) && (!bAttacking))
	{
		bMoveForward = true;
		//Cameraboom�� ControlRotation�� �̿�. ControlRotation�� �̿��ؼ� ȸ���������� ����.
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		//�� YawRotation������ ȸ������� ����� �̷κ��� X���� ��´�. (ȸ��ü�� ���ع������� Forward����(x)�� ����)
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::MoveRight(float Value)
{
	bMoveRight = false;
	if ((Controller != nullptr) && (Value != 0.f) && (!bAttacking))
	{
		bMoveRight = true;
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::TurnAtRate(float Rate)
{
	//�� frame���� BaseRate��ŭ ȸ���Ѵ�.
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


void AMainCharacter::CharacterRotate()
{
	float ForwardAxis = GetInputAxisValue(FName("MoveForward"));
	float RightAxis = GetInputAxisValue(FName("MoveRight"));
	FVector Direction = FVector(ForwardAxis, RightAxis, 0.f).GetSafeNormal(); //�Է¿� ���� ����.

	FRotator Rotation = FRotator(0.f, Controller->GetControlRotation().Yaw + Direction.Rotation().Yaw, 0.f); //ȸ������ �����࿡ �Է¹����� ����
	
	SetActorRotation(Rotation);
}

/************ Money **************/
///////// Coin, Soul ���� �Լ� /////
/*********************************/
void AMainCharacter::IncrementCoin(int32 Amount)
{
	Coins += Amount;
}


/*************** Damage ****************/
//////////   Damage ���� �Լ�   /////////
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
///////// Weapon ���� �Լ� /////
/*********************************/
void AMainCharacter::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	if (EquippedWeapon) //���� �����Ȱ� ������
	{
		EquippedWeapon->Destroy(); //destory�ϰ� ����.
	}
	EquippedWeapon = WeaponToSet;
}

/************ Combat **************/
///////// Combat ���� �Լ� /////
/*********************************/
void AMainCharacter::Attack()
{
	bool fall = GetCharacterMovement()->IsFalling();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if (fall && CalcAirAttack()) //���߿� ������ AttackAir�Լ���
	{
		AttackAir();
		return;
	}

	if (AnimInstance && CombatMontage)
	{
		bAttacking = true;
		
		CharacterRotate(); //������ ȸ���Է��� ������ ȸ���ϵ��� ����.

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

//���߿� MainChar�� Stats�� weapon���� �Ѱ������.
void AMainCharacter::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString MapName = World->GetMapName();

		FName CurrentLevelName(*MapName);

		//������ Level�� ����Level�� �ٸ��� Open world�� �Ѵ�.
		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}