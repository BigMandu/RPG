// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Weapon.h"
#include "Enemy.h"
#include "MainPlayerController.h"
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

	//���� ������ ����//
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

	//ĳ������ ���� ����
	{
		/***Line Trace (ĳ���� �ؿ������� Mesh������ �Ÿ����� ****/
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

		//���� ������
		float AfterHeight = 0.f;
		if (GetCharacterMovement()->IsFalling()) //���߿��� �������� Damage�� ��� �� ����.
		{
			AfterHeight = 0.f;
			FallingDamageCalc();
		}
		else if (GetCharacterMovement()->IsFalling() == false)
		{
			AfterHeight = OutHit.ImpactPoint.Z;
			TakeFallingDamage(AfterHeight);
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

	PlayerInputComponent->BindAction("RMB", EInputEvent::IE_Pressed, this, &AMainCharacter::RMBDown);
	PlayerInputComponent->BindAction("RMB", EInputEvent::IE_Released, this, &AMainCharacter::RMBUp);

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

void AMainCharacter::RMBDown()
{
	if (EquippedWeapon && !bAttacking)
	{
		FName AttachedSocketName = EquippedWeapon->GetAttachParentSocketName();
		//UE_LOG(LogTemp, Warning, TEXT("Weapon Attach at %s"), *(AttachedSocketName.ToString()));
		if (AttachedSocketName != NAME_None)
		{
			EquippedWeapon->ThrowWeapon(this, AttachedSocketName);
			//EquippedWeapon->SetWeaponOwner(nullptr);
			EquippedWeapon = nullptr;
			//FVector WeaponThrow = FMath::VInterpTo(EquippedWeapon->GetActorLocation(), Destination, GetWorld()->GetDeltaSeconds(), 10.f);
			

		}
	}
}

void AMainCharacter::RMBUp()
{
	
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
	if ((PlayerController != nullptr) && (Value != 0.f) && (!bAttacking))
	{
		bMoveForward = true;
		//Cameraboom�� ControlRotation�� �̿�. ControlRotation�� �̿��ؼ� ȸ���������� ����.
		const FRotator Rotation = PlayerController->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		//�� YawRotation������ ȸ������� ����� �̷κ��� X���� ��´�. (ȸ��ü�� ���ع������� Forward����(x)�� ����)
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


void AMainCharacter::CharacterRotate() //���ݶ� �����.
{
	float ForwardAxis = GetInputAxisValue(FName("MoveForward"));
	float RightAxis = GetInputAxisValue(FName("MoveRight"));
	FVector Direction = FVector(ForwardAxis, RightAxis, 0.f).GetSafeNormal(); //�Է¿� ���� ����.

	
	FRotator Rotation = FRotator(0.f, PlayerController->GetControlRotation().Yaw + Direction.Rotation().Yaw, 0.f); //ȸ������ �����࿡ �Է¹����� ����

	SetActorRotation(Rotation);
}

/************ Money **************/
///////// Coin, Soul ���� �Լ� /////
/*********************************/
void AMainCharacter::IncrementCoin(int32 Amount)
{
	Coins += Amount;
}

void AMainCharacter::IncrementSoul(int32 Amount)
{
	Souls += Amount;
}

/*************** Damage ****************/
//////////   Damage ���� �Լ�   /////////
/***************************************/

//���⸦ �̿��� ����, Weapon Class���� ȣ����.
void AMainCharacter::AttackGiveDamage(AEnemy* DamagedEnemy, float WeaponDamage) //������ Damage�� ��.
{
	
	UGameplayStatics::ApplyDamage(DamagedEnemy, PlayerDamage + WeaponDamage, GetController(), this, DamageTypeClass);

	//UE_LOG(LogTemp, Warning, TEXT("MainPlayer->AttackDamage()"));
	//UE_LOG(LogTemp, Warning, TEXT("Player Base Damage is : %f, EquippedWeapon Damage is : %f"), PlayerDamage, WeaponDamage);
	//UE_LOG(LogTemp, Warning, TEXT("Total Damage is : %f"), PlayerDamage + WeaponDamage);
}

//��������, AnimNotifyState_RangeAttack���� ȣ����( �ִ� ��Ƽ���� ������Ʈ)
void AMainCharacter::AttackRangeDamage() //Player�� ���� ���� (��ų ������.)
{
	//UE_LOG(LogTemp, Warning, TEXT("MainCharacter::AttackRangeDamage()"));
	FCollisionQueryParams Params(FName(TEXT("PlayerRangeDamage")), false, this);
	TArray<FHitResult>OutHit;
	FVector StartLocation = GetActorLocation(); //StartLocation
	FVector WeaponLength = EquippedWeapon->CombatCollision->GetScaledBoxExtent() * 2.0f;

	FVector EndLocation = GetActorForwardVector() * WeaponLength.Z + StartLocation;

	//Weapon�� CombatCollision * 2 ũ�⸦ ���� ĸ��������� �����Ѵ�.
	GetWorld()->SweepMultiByChannel(OutHit, StartLocation, EndLocation, FQuat::Identity, 
		ECollisionChannel::ECC_Pawn, FCollisionShape::MakeCapsule(WeaponLength), Params);
	
	float Damage = PlayerDamage;
	bool AirAttack = false;
	if (GetCharacterMovement()->IsFalling()) //�������� ������, ���ϳ��̿� 15%�� �������� �� ��.
	{
		AirAttack = true;
		Damage += CurHeight * 0.15f;
		//UE_LOG(LogTemp, Warning, TEXT("Damage is : %f, Weapon Damage is : %f, TotalDamage is : %f"), Damage, EquippedWeapon->WeaponDamage, Damage + EquippedWeapon->WeaponDamage);
	}

	float ZSize = WeaponLength.Z;
	float YSize = WeaponLength.Y;
	float XSize = WeaponLength.X;

	//������
	{
		DrawDebugCapsule(GetWorld(), EndLocation,
			ZSize * 0.5 + XSize + YSize, XSize + YSize, FRotationMatrix::MakeFromZ(GetActorForwardVector() * ZSize).ToQuat(),
			FColor::Red, false, 2.0f);
	}

	if (OutHit.Num() == 0) return;

	for (auto Hit : OutHit)
	{
		AEnemy* Enemy = Cast<AEnemy>(Hit.GetActor());
		if (Enemy)
		{
			//������
			{
				DrawDebugCapsule(GetWorld(), EndLocation,
					ZSize * 0.5 + XSize + YSize, XSize + YSize, FRotationMatrix::MakeFromZ(GetActorForwardVector() * ZSize).ToQuat(),
					FColor::Green, false, 2.0f);
				//UE_LOG(LogTemp, Warning, TEXT("Player Range Attack success, Damage is : %f, Weapon Damage is : %f, TotalDamage is : %f"), Damage, EquippedWeapon->WeaponDamage, Damage + EquippedWeapon->WeaponDamage);
			}
			if (AirAttack == true)
			{
				//Air Attack�̸� ���� �������� ���ڿ� Damage���� �ϱ�.
			}
			UGameplayStatics::ApplyDamage(Enemy, Damage + EquippedWeapon->WeaponDamage, GetController(), this, DamageTypeClass);
		}
	}
	
}

void AMainCharacter::FallingDamageCalc() //���Ͻ� �ִ� ���̸� ���Ѵ�.
{
	if (FallingMaxHeight < CurHeight)
	{
		FallingMaxHeight = CurHeight;
	}	
}

void AMainCharacter::TakeFallingDamage(float AfterHeight)
{
	//�������� ���� ���Ŀ� ������ ���, ���� �� �ʱ�ȭ�� ���ش�.
	
	 // UKismetMathLibrary::Abs(FallingMaxHeight - AfterHeight)
	if (FallingMaxHeight-AfterHeight >= GetDefaultHalfHeight() * 3.f) //�ڱ�Ű��  1.5�谡 �Ǹ� ���ϵ������� �޴´�. 
	{
		FallingDamage = FallingMaxHeight * 0.05f; //���̿��� 5%�� �������� �ش�.
		DecrementHealth(FallingDamage);
		/*UE_LOG(LogTemp, Warning, TEXT("FMH is : %f, AfterHeight is : %f"), FallingMaxHeight, AfterHeight);
		UE_LOG(LogTemp, Warning, TEXT("Subtract is : %f"), FallingMaxHeight - AfterHeight);
		UE_LOG(LogTemp, Warning, TEXT("Falling Damage is : %f"), FallingDamage);*/
	}
	//���� ���� �ʱ�ȭ.
	FallingDamage = 0.f;
	FallingMaxHeight = 0.f;

}

float AMainCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	//UE_LOG(LogTemp, Warning, TEXT("MainCharacter::TakeDamage()::::Damage Causer : %s , Damage : %f"), *DamageCauser->GetName(), DamageAmount);
	DecrementHealth(DamageAmount);
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
///////// Weapon ���� �Լ� /////
/*********************************/
void AMainCharacter::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	if (EquippedWeapon) //���� �����Ȱ� ������
	{
		EquippedWeapon->Destroy(); //destory�ϰ� ����, ���߿� Inventory�� �־�����.
	}
	EquippedWeapon = WeaponToSet;
	EquippedWeapon->SetWeaponOwner(this);
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
	else if (MovementStatus == EMovementStatus::EMS_Sprint) //������Ʈ �����϶� �뽬����
	{
		SptirntAttack();
		return;
	}

	//���߰���, �뽬������ �켱������ �ϰ� ����/ �Ϲ� ����.

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
				
				//�����
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
					
					AddMovementInput(TargetVector, 1.0f * GetVelocity().Size());//ĳ���� �ӵ� ������ ���� ������ ������.
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
	AttackCount = 0;
	if (GetWorldTimerManager().IsTimerActive(SprintAttackTimer))
	{
		//GetCapsuleComponent()->SetCapsuleHalfHeight(95.f);
		GetWorldTimerManager().ClearTimer(SprintAttackTimer);
	}
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