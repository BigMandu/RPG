// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
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

	/*******************************/
	//-- Player Movement Status ---//
	/*******************************/

	////������ (����) ���� ////
	GetCharacterMovement()->bOrientRotationToMovement = true; //������ ���� = ����������� ����
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f); //���� ȸ���ӵ���.
	GetCharacterMovement()->JumpZVelocity = 450.f; //���� ���� ����.
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
	////////////////////////////
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
	
	//StaminaStatus ����.
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
		else //Shift Key�� ������ ������
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
		else //Shitkey�� �ȴ�������
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
		if (bShiftKeyDown) //���������� �ٷ� �Ѱ��ش�.
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

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
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
		Attack();
	}
}

void AMainCharacter::LMBUp()
{
	bLMBDown = false;
}


/*************** Movement **************/
//////////   Movement ���� �Լ�  ////////
/**************************************/

void AMainCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.f) && (!bAttacking))
	{
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
	bAttacking = true;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);

		AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
	}
}

void AMainCharacter::AttackEnd()
{
	bAttacking = false;
}