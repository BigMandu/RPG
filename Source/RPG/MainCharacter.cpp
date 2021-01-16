// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Weapon.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "Explosive.h"
#include "Store.h"
#include "SaveGameCustom.h"
#include "ItemSave.h"
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
#include "Components/AudioComponent.h"



// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	////////////TEST AI/////////////
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));

	//Load�Ҷ� FirstMap�� ���ļ� �ε��� �Ǳ⶧���� �̸� üũ��.
	bFirstLoad = false;

	//Smash Ability����� ��򰡿� �ε����� �ٷ� ���߰� �ϱ� ���ؼ�.
	bCapsuleHit = false;

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
	bLMBDown = false;
	bRMBDown = false;
	bShiftKeyDown = false;
	bEKeyDown = false;
	bShiftKeyDown = false;
	bJumpKeyDown = false;
	bMoveForward = false;
	bMoveRight = false;
	bFkeyDown = false;

	bClickRMB = false;

	/*******************************/
	//-- Player Movement  ---//
	/*******************************/

	bJumped = false;

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
	FallingMaxHeight = -999.f;
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

	ThrowAbility_Distance = 800.f;
	ThrowAbility_Rotation = 1024.f;
	SmashAbility_Damage = 200.f;

	/////////////////////////
	//Ability cool down
	ThrowWeaponCooldown = 3.f;
	SmashCooldown = 5.f;

	
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
	
	//bSwitchLevel = false;
	PlayerController = Cast<AMainPlayerController>(GetController());

	////////////TEST AI/////////////
	StimuliSourceComponent->bAutoRegister = true;
	StimuliSourceComponent->RegisterForSense(SenseSight); //Sight Sense�� ���.


	//ĸ��������Ʈ�� �� ���� �Լ�
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AMainCharacter::CapsuleOnHit);


	//Abiltiy cooldown�� ����.
	ThrowTick = ThrowWeaponCooldown;
	SmashTick = SmashCooldown;

	bCanThrow = true;
	bCanSmash = true;
	///Beginplay�� �ʱ�ȭ�� ���༭ level transition�̳�, �ε��Ҷ� �ٷ� ����� �� �ֵ��� ���ش�.

	/// �ʷε��� ���� �۾�
	FString CurMapName = GetWorld()->GetMapName();
	CurMapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *CurMapName);

	if (CurMapName == TEXT("FrozenCove"))
	{
		if (LevelFrozenCaveSound)
		{
			LevelAudioComponent = UGameplayStatics::CreateSound2D(this, LevelFrozenCaveSound);
			LevelAudioComponent->Play();
				
		}
	}

	if (CurMapName == TEXT("Demonstration"))
	{
		if (LevelDungeonSound)
		{
			LevelAudioComponent = UGameplayStatics::CreateSound2D(this, LevelDungeonSound);
			LevelAudioComponent->Play();
		}
	}



	if (CurMapName != TEXT("Firstmap"))
	{
		if (CurMapName == TEXT("Titlemap")) return;
		USaveGameCustom* LoadGameInst = Cast<USaveGameCustom>(UGameplayStatics::CreateSaveGameObject(USaveGameCustom::StaticClass()));
		LoadGameInst = Cast<USaveGameCustom>(UGameplayStatics::LoadGameFromSlot(LoadGameInst->SlotName, LoadGameInst->UserIndex));
		if (LoadGameInst)
		{
			if (LoadGameInst->SaveCharacterStats.bFirstLoadGame == true)
			{
				LoadGameInst->SaveCharacterStats.bFirstLoadGame = false;
				UGameplayStatics::SaveGameToSlot(LoadGameInst,LoadGameInst->SlotName, LoadGameInst->UserIndex);

				LoadGame(false);
				SaveGame(true); //SaveGame�� bool���� ���� �ǹ� ����.
			}
			else
			{
				LoadGame(true); //������ ����, ���ݵ��� �ε��Ѵ�.
				SaveGame(true); //�׸��� �ٽ� �����Ѵ�.
			}
		}
		
	}
	else if (CurMapName == TEXT("Firstmap"))
	{
		//LoadGame(false);
		LoadGame_FirstLoad();
	}

}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//���� ���¸� �׳� ����.
	if (MovementStatus == EMovementStatus::EMS_Dead)
		return;

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
		FVector EndPoint = FVector(StartPoint.X, StartPoint.Y, -990.f);

		FCollisionQueryParams CollisionParams;
		//DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 1.f, 0, 2); //Line Trace �ð�ȭ Debug

		bool isHit = GetWorld()->LineTraceSingleByChannel(OutHit, StartPoint, EndPoint, ECollisionChannel::ECC_Visibility, CollisionParams);

		if (isHit)
		{
			if (OutHit.bBlockingHit)
			{
				CurHeight = StartPoint.Z - OutHit.ImpactPoint.Z;
				//������
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
			FallingDamageCalc(StartPoint.Z); //���Ͻ� �ִ� ���̸� ���ϴ��Լ�.
			bJumped = true;
		}
		else if (GetCharacterMovement()->IsFalling() == false && bJumped == true)
		{
			//AfterHeight = OutHit.ImpactPoint.Z; //������ ���� ���̸� �Ѱ��ش�.
			AfterHeight = OutHit.ImpactPoint.Z;
			TakeFallingDamage(AfterHeight);
			bJumped = false;
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

	PlayerInputComponent->BindAction("ESC", EInputEvent::IE_Pressed, this, &AMainCharacter::ESCKeyDown);
	PlayerInputComponent->BindAction("ESC", EInputEvent::IE_Released, this, &AMainCharacter::ESCKeyUp);
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
//////////   Input ���� �Լ�  ////////
/**************************************/
void AMainCharacter::ESCKeyDown()
{
	bESCKeyDown = true;
	if (PlayerController)
	{
		PlayerController->TogglePauseMenu();

		if (PlayerController->bIsStorePageVisible == true)
		{
			PlayerController->RemoveStorePage();
		}		
	}
}

void AMainCharacter::ESCKeyUp()
{
	bESCKeyDown = false;
}

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
	if (OverlappingActor)
	{
		AWeapon* Weapon = Cast<AWeapon>(OverlappingActor);
		AStore* Store = Cast<AStore>(OverlappingActor);
		if(Weapon)
		{ 
			Weapon->Equip(this);
		}
		else if (Store)
		{
			UE_LOG(LogTemp, Warning, TEXT("Store overlapping"));
			PlayerController->DisplayStorePage();
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
	bClickRMB = true;
	if (Ability_ThrowWeapon_Cooldown_Check())
	{
		Ability_ThrowWeapon_Before();
	}	
}

void AMainCharacter::RMBUp()
{
	bRMBDown = false;
	bAttacking = false;
	if(bCanThrow)
	{
		bCanThrow = false;
		Ability_ThrowWeapon();
	}	
}

void AMainCharacter::Jump()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead)
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
	if (Ability_Smash_Cooldown_Check())
	{
		Ability_Smash();
	}
	
}

void AMainCharacter::FKeyUp()
{
	bFkeyDown = false;
}

//CapsuleComponent �浹 ����
void AMainCharacter::CapsuleOnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Hit.bBlockingHit && bAttacking)
	{
		bCapsuleHit = true;
		//UE_LOG(LogTemp, Warning, TEXT("Capsule hit! %s"), *(Hit.Actor->GetFName().ToString()));
	}
}


/*************** Movement **************/
//////////   Movement ���� �Լ�  ////////
/**************************************/

void AMainCharacter::MoveForward(float Value)
{
	bMoveForward = false;
	if ((PlayerController != nullptr) && (Value != 0.f) && (!bAttacking) && MovementStatus != EMovementStatus::EMS_Dead)
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
	if ((PlayerController != nullptr) && (Value != 0.f) && (!bAttacking) && MovementStatus != EMovementStatus::EMS_Dead)
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
///////// Pickup ���� �Լ� /////
/*********************************/
void AMainCharacter::IncrementCoin(int32 Amount)
{
	Coins += Amount;
}

void AMainCharacter::IncrementSoul(int32 Amount)
{
	Souls += Amount;
}

void AMainCharacter::DecrementCoin(int32 Amount)
{
	Coins -= Amount;
}

void AMainCharacter::DecrementSoul(int32 Amount)
{
	Souls -= Amount;
}

void AMainCharacter::SetMaxHealthPoint(float MaxHP)
{
	MaxHealth = MaxHP;
}

void AMainCharacter::SetMaxStamina(float MaxStat)
{
	MaxStamina = MaxStat;
}

void AMainCharacter::SetPlayerDamage(float CharDamage)
{
	PlayerDamage = CharDamage;
}

void AMainCharacter::SetRMBDistance(float RMBDistance)
{
	ThrowAbility_Distance = RMBDistance;
}

void AMainCharacter::SetRMBRotation(float RMBRotation)
{
	ThrowAbility_Rotation = RMBRotation;
}

void AMainCharacter::SetFDamage(float FDamage)
{
	SmashAbility_Damage = FDamage;
}


void AMainCharacter::IncrementHealth(float Amount)
{
	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health += Amount;
	}
	
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

	//Damage���� Ȯ��.
	float Damage = PlayerDamage;
	if (GetCharacterMovement()->IsFalling()) //�������� ������, ���ϳ��̿� 15%�� �������� �� ��. 
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

	//������ ����
	FVector WeaponLength = EquippedWeapon->CombatCollision->GetScaledBoxExtent() * 2.0f;
	float ZSize = WeaponLength.Z;
	float YSize = WeaponLength.Y;
	float XSize = WeaponLength.X;


	//��ų ���� EndLocation, Collision�� �ٸ��� �������ش�.
	if (bAbilitySmash)
	{
		
		EndLocation = GetActorLocation();
		CollisionChannel = ECollisionChannel::ECC_WorldDynamic; //WorldDynamic ��Ҹ� ����(���δٸ���)
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

		//Weapon�� CombatCollision * 2 ũ�⸦ ���� ĸ��������� �����Ѵ�.	
		GetWorld()->SweepMultiByChannel(OutHit, StartLocation, EndLocation, FQuat::Identity,
			CollisionChannel, CollisionShape, Params, ResponseParams);
	}

	//������
	/*{
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
	}*/
	

	if (OutHit.Num() == 0) return;

	for (auto Hit : OutHit)
	{
		AEnemy* Enemy = Cast<AEnemy>(Hit.GetActor());
		AExplosive* Explosive = Cast<AExplosive>(Hit.GetActor());
		if (bAbilitySmash)
		{
			if (Enemy)
			{
				/*UE_LOG(LogTemp, Warning, TEXT("victim name is %s"), *(Hit.Actor->GetFName().ToString()));
				UE_LOG(LogTemp, Warning, TEXT("RangeAttack AbilitySmash Apply Damage"));*/
				TArray<AActor*>IgnoreActor;

				UGameplayStatics::ApplyRadialDamage(this, SmashAbility_Damage + EquippedWeapon->WeaponDamage, GetActorLocation(), 600.f, DamageTypeClass, IgnoreActor, this, PlayerController, true,
					ECollisionChannel::ECC_WorldStatic); //������ ���ڰ���, Ÿ�� ������ ���ͻ��̿� �ش� ä���� ���� ���Ͱ� �ִٸ� damamge�� ���� �ʴ´ٴ°���.

				//������
				//DrawDebugSphere(GetWorld(), GetActorLocation(), 400.f, int32(12), FColor::Green, false, 2.f, (uint8)nullptr, 1.0f);
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
				//UE_LOG(LogTemp, Warning, TEXT("RangeAttack Apply Damage"));
				UGameplayStatics::ApplyDamage(Enemy, Damage + EquippedWeapon->WeaponDamage, PlayerController, this, DamageTypeClass);
				//������
				{
					/*DrawDebugCapsule(GetWorld(), EndLocation,
						ZSize * 0.5 + XSize + YSize, XSize + YSize, FRotationMatrix::MakeFromZ(GetActorForwardVector() * ZSize).ToQuat(),
						FColor::Green, false, 2.0f);*/
					//UE_LOG(LogTemp, Warning, TEXT("Player Range Attack success, Damage is : %f, Weapon Damage is : %f, TotalDamage is : %f"), Damage, EquippedWeapon->WeaponDamage, Damage + EquippedWeapon->WeaponDamage);
				}
			}
		}
	}
}

void AMainCharacter::FallingDamageCalc(float JumpHeight) //���Ͻ� �ִ� ���̸� ���Ѵ�.
{
	if (FallingMaxHeight < JumpHeight)
	{
		FallingMaxHeight = JumpHeight; //feet location.z
	}
}

void AMainCharacter::TakeFallingDamage(float AfterHeight) //outhit impact.z
{
	//�������� ���� ���Ŀ� ������ ���, ���� �� �ʱ�ȭ�� ���ش�.
	
	if (bAbilitySmash == true) //�ش� ability�� �����(������)�̸� ���ϵ��� ����.
	{
		FallingMaxHeight = -999.f;
		return;
	}
	//���������� �ִ���̿� ������ ��������� ���̰� �ڱ�Ű�� 1.5�谡 ������ �������� �ް� �ߴ�. -> ���������� ���������� �����Ҷ� ������ �޴°� �����ϱ� ���ؼ�.
	if ( FallingMaxHeight-AfterHeight >= GetDefaultHalfHeight() * 3.f) //�ڱ�Ű��  1.5�谡 �Ǹ� ���ϵ������� �޴´�. 
	{
		FallingDamage = (FallingMaxHeight - AfterHeight) * 0.05f; //������ ���̿��� 5%�� �������� �ش�.
		DecrementHealth(FallingDamage);
		//������
		/*UE_LOG(LogTemp, Warning, TEXT("FMH is : %f, AfterHeight is : %f"), FallingMaxHeight, AfterHeight);
		UE_LOG(LogTemp, Warning, TEXT("Subtract is : %f"), FallingMaxHeight - AfterHeight);
		UE_LOG(LogTemp, Warning, TEXT("Falling Damage is : %f"), FallingDamage);*/
	}
	/*UE_LOG(LogTemp, Warning, TEXT("FMH is : %f, AfterHeight is : %f"), FallingMaxHeight, AfterHeight);
	UE_LOG(LogTemp, Warning, TEXT("Subtract is : %f"), FallingMaxHeight - AfterHeight);*/
	//���� ���� �ʱ�ȭ.
	FallingDamage = 0.f;
	FallingMaxHeight = -999.f;

}

float AMainCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	//UE_LOG(LogTemp, Warning, TEXT("MainCharacter::TakeDamage()::::Damage Causer : %s , Damage : %f"), *DamageCauser->GetName(), DamageAmount);
	
	if (bAbilitySmash == false) //�ش� �����Ƽ ���� Damage�� ���� �ʱ� �ϱ� ����.
	{
		if (HitSound)
		{
			UGameplayStatics::PlaySound2D(this, HitSound);
		}
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
	//UE_LOG(LogTemp, Warning, TEXT("AMainCharacter::Die()"));
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	SetMovementStatus(EMovementStatus::EMS_Dead);
	Health = 0.f;
	Stamina = 0.f;

	UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
	if (AnimIns && DeathMontage)
	{
		AnimIns->Montage_Play(DeathMontage, 1.f);
		AnimIns->Montage_JumpToSection(TEXT("Execute"), DeathMontage);
	}
}

void AMainCharacter::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
	GetWorldTimerManager().ClearAllTimersForObject(this);
	PlayerController->DisplayPauseMenu();
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
			//UE_LOG(LogTemp, Warning, TEXT("Land Crash Attack // CurHeight : %f"), CurHeight);
		}
		else
		{
			AnimInstance->Montage_JumpToSection(FName("AirAttack_1"), AirCombatMontage);
			//UE_LOG(LogTemp, Warning, TEXT("Air Attack // CurHeight : %f"), CurHeight);
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
	bCapsuleHit = false; //smash ability������ capsule hit�� Ǯ���ֱ� ����.
	bAbilitySmash = false;
	SetCanBeDamaged(true); //smash ability������ ������ Ǯ���ֱ�����.

	AttackCount = 0;
	if (GetWorldTimerManager().IsTimerActive(SprintAttackTimer))
	{
		//GetCapsuleComponent()->SetCapsuleHalfHeight(95.f);
		GetWorldTimerManager().ClearTimer(SprintAttackTimer);
	}
}

void AMainCharacter::Ability_ThrowWeapon_Before()
{
	if (bRMBDown)
	{
		//if (EquippedWeapon && !bAttacking)
		{
			bAttacking = true;
			UAnimInstance* Anim = GetMesh()->GetAnimInstance();
			if (Anim && AbilityThrowWeaponMontage)
			{				
				GetWorldTimerManager().SetTimer(RMBDownTimerHandle, [=] {
					if (bRMBDown)
					{
						FRotator Rotation = PlayerController->GetControlRotation();
						FRotator RotationYaw = FRotator(0.f, Rotation.Yaw, 0.f);
						SetActorRotation(RotationYaw);
						Anim->Montage_Play(AbilityThrowWeaponMontage, 1.f);
						Anim->Montage_JumpToSection(FName("Ready"), AbilityThrowWeaponMontage);
					}
				}, GetWorld()->GetDeltaSeconds(), true);

				

				
			}
		}
	}
}

void AMainCharacter::Ability_ThrowWeapon()
{
	if (bClickRMB)
	{
		bClickRMB = false;
		if (EquippedWeapon && !bAttacking)
		{
			FName AttachedSocketName = EquippedWeapon->GetAttachParentSocketName();
			//UE_LOG(LogTemp, Warning, TEXT("Weapon Attach at %s"), *(AttachedSocketName.ToString()));
			if (AttachedSocketName != NAME_None)
			{
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if (AnimInstance && AbilityThrowWeaponMontage)
				{
					if (GetWorldTimerManager().IsTimerActive(RMBDownTimerHandle))
					{
						GetWorldTimerManager().ClearTimer(RMBDownTimerHandle);
					}

					AnimInstance->Montage_Play(AbilityThrowWeaponMontage, 2.25f);
					AnimInstance->Montage_JumpToSection(FName("Execute"), AbilityThrowWeaponMontage);

					EquippedWeapon->ThrowWeapon(this, AttachedSocketName, ThrowAbility_Distance, ThrowAbility_Rotation);
					EquippedWeapon = nullptr;
					//UE_LOG(LogTemp, Warning, TEXT("Main : AbilityRotation : %f"), ThrowAbility_Rotation);					
				}
			}
		}
	}
}

void AMainCharacter::Ability_ThrowWeapon_Finish() //AWeapon::ReceiveWeapon���� ȣ��
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AbilityThrowWeaponMontage)
	{
		AnimInstance->Montage_Play(AbilityThrowWeaponMontage, 1.5f);
		AnimInstance->Montage_JumpToSection(FName("Finish"), AbilityThrowWeaponMontage);

		//���� �� ���� ��Ÿ�� ����.
		GetWorldTimerManager().SetTimer(ThrowWeaponCooldownHandle, this, &AMainCharacter::Ability_ThrowWeapon_Cooldown, GetWorld()->GetDeltaSeconds(), true);
	}
}

void AMainCharacter::Ability_Smash()
{
	//if (EquippedWeapon && AbilitySmashMontage && !bAttacking && !GetCharacterMovement()->IsFalling()) //Cooldown check���� �ع�����.
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			bAttacking = true;
			bAbilitySmash = true;

			FVector OriginLocation = GetActorLocation();
			FVector TargetLocation = GetActorForwardVector() * 400.f;
			TargetLocation.Z = 320.f; //Ÿ�ٸ������� ���̸� ��������.

			FVector LastLocation = OriginLocation + TargetLocation;

			AnimInstance->Montage_Play(AbilitySmashMontage, 1.4f);
			AnimInstance->Montage_JumpToSection(FName("Execute"), AbilitySmashMontage);

			SetCanBeDamaged(false); //�ش� ��ų ����� �������¸� �ο��Ѵ�.
		
			GetWorldTimerManager().SetTimer(AbilitySmashHandle, [=]{

				bool bIsNear = FVector::PointsAreNear(GetActorLocation(), LastLocation, 80.f); //near����
				bool bAnimPlay = AnimInstance->Montage_IsActive(AbilitySmashMontage); //anim ��� ����

				GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); //�ش� ��ų ���� pawn�� �����Ѵ� ->�����̵�����

				//������
				//DrawDebugSphere(GetWorld(), LastLocation, 25.f, 6, FColor::Green, false, 4.f, (uint8)nullptr, 1.0f);

				if (bIsNear || bAnimPlay == false || bCapsuleHit == true) //bAnimplay�� Ȥ�ø� ���׶����� �־�״�.
				{
					Ability_Smash_Finish();
				}
				else
				{
					GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying); //�̷��� ���ִϱ� ��¥ �ε巴�� �����δ�. -> �ٷ��� �̷��� �ص־���.
					AddMovementInput(TargetLocation, 1.0f, true);
				}
				}, GetWorld()->GetDeltaSeconds(), true);
		}
	}
}

void AMainCharacter::Ability_Smash_Finish() //���߿� �����ѵ� Ǯ���ִ°�.
{
	//bCapsuleHit = false;
	GetWorldTimerManager().ClearTimer(AbilitySmashHandle);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);


	//������ ���� ��Ÿ���� ������.
	GetWorldTimerManager().SetTimer(SmashCooldownHandle, this, &AMainCharacter::Ability_Smash_Cooldown, GetWorld()->GetDeltaSeconds(), true);
}


//**************************************************//
//				Ability Cool down ����				//
//**************************************************//

bool AMainCharacter::Ability_ThrowWeapon_Cooldown_Check()
{
	//ThrowWeaponCooldown = 3.f; �ʱⰪ
	if (EquippedWeapon && !bAttacking) //���⼭ ������ �Ѵ�.
	{
		//UE_LOG(LogTemp, Warning, TEXT("Check:: ThrowTick is : %.2f"), ThrowTick);

		if (ThrowTick >= ThrowWeaponCooldown)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Check:: ThrowTick clear"));
			ThrowTick = 0.f;
			return true;
		}
		else return false;
	}
	else return false;
}

void AMainCharacter::Ability_ThrowWeapon_Cooldown() //Ability_ThrowWeapon_Finish���� Timer�� ȣ���Ѵ�.
{
	ThrowTick += GetWorld()->GetDeltaSeconds();
	//UE_LOG(LogTemp, Warning, TEXT("Cooldown:: ThrowTick is : %.2f"), ThrowTick);
	if (ThrowTick >= ThrowWeaponCooldown)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Cooldown:: Value is Equal, ThrowTick : %.2f, SmashCooldown : %.2f"), ThrowTick, ThrowWeaponCooldown);
		GetWorldTimerManager().ClearTimer(ThrowWeaponCooldownHandle);
		bCanThrow = true;
		return;
	}
}


bool AMainCharacter::Ability_Smash_Cooldown_Check()
{
	//SmashCooldown = 5.f; �ʱⰪ

	//���⼭ �ƿ� ������ �ع�����
	if (EquippedWeapon && AbilitySmashMontage && !bAttacking && !GetCharacterMovement()->IsFalling())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Check:: SmashTick is : %.2f"), SmashTick);

		if (SmashTick >= SmashCooldown)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Check:: SmashTick clear"));
			SmashTick = 0.f;
			return true;
		}
		else return false;
	}
	else return false;
}



void AMainCharacter::Ability_Smash_Cooldown() //Ability_Smash_Finish���� Timer�� ȣ���Ѵ�.
{
	SmashTick += GetWorld()->GetDeltaSeconds();
	//UE_LOG(LogTemp, Warning, TEXT("Cooldown:: SmashTick is : %.2f"), SmashTick);
	if (SmashTick >= SmashCooldown)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Cooldown:: Value is Equal, SmashTick : %.2f, SmashCooldown : %.2f"), SmashTick, SmashCooldown);
		GetWorldTimerManager().ClearTimer(SmashCooldownHandle);
		bCanSmash = true;
		return;
	}
}
/*******************************/
//--- Game Level,Save,Load  ---//
/*******************************/
void AMainCharacter::SwitchLevel(FName LevelName) //Level Transition Volume�� �������Ǹ� ȣ���
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString MapName = World->GetMapName();
		FName CurrentLevelName(*MapName);

		//������ Level�� ����Level�� �ٸ��� Open world�� �Ѵ�.
		if (CurrentLevelName != LevelName)
		{
			SaveGame(true); //���� ��ȯ�ÿ� ������ ����� ���⸦ ����´�.

			UGameplayStatics::OpenLevel(World, LevelName); //Open level�ϸ� beginplay�� �����.
		}
	}
}

void AMainCharacter::SaveGame(bool bSwitchLevel)
{
	/*CreateSaveGameObject�� SaveGame class�� ���ڷ� �ް�, SaveGame�� �����Ѵ�.
	* ���� �׳� USaveGameCustom* Savegame = Cast<USaveGameCustom>(����) �̷��� �ص� ������ 
	* �������� ���� �Ʒ� ó�� �غô�.
	* 
	* USaveGameCustom class�� ���� SaveGameObject�� �����Ѱ��� SaveGame�� �����̵ư�
	* USaveGameCustom�� ���������� ������ ������ class�� ĳ��Ʈ�ؼ� �־����.
	*/
	
	USaveGame* SaveGame = UGameplayStatics::CreateSaveGameObject(USaveGameCustom::StaticClass());
	USaveGameCustom* SaveGameInstance = Cast<USaveGameCustom>(SaveGame);
	if (SaveGameInstance) //�� ������ ��ü�ȿ� �ִ� ����ü�� ������ �������� �����Ѵ�.
	{
		SaveGameInstance->SaveCharacterStats.Health = Health;
		SaveGameInstance->SaveCharacterStats.MaxHealth = MaxHealth;
		SaveGameInstance->SaveCharacterStats.Stamina = Stamina;
		SaveGameInstance->SaveCharacterStats.MaxStamina = MaxStamina;
		SaveGameInstance->SaveCharacterStats.Coins = Coins;
		SaveGameInstance->SaveCharacterStats.Souls = Souls;
		SaveGameInstance->SaveCharacterStats.PlayerDamage = PlayerDamage;
		SaveGameInstance->SaveCharacterStats.ThrowAbility_Distance = ThrowAbility_Distance;
		SaveGameInstance->SaveCharacterStats.ThrowAbility_Rotation = ThrowAbility_Rotation;
		SaveGameInstance->SaveCharacterStats.SmashAbility_Damage = SmashAbility_Damage;


		SaveGameInstance->SaveCharacterStats.HealthPurButtonCount = HealthPurButtonCount;
		SaveGameInstance->SaveCharacterStats.StaminaPurButtonCount = StaminaPurButtonCount;
		SaveGameInstance->SaveCharacterStats.DamagePurButtonCount = DamagePurButtonCount;
		SaveGameInstance->SaveCharacterStats.RMBDistancePurButtonCount = RMBDistancePurButtonCount;
		SaveGameInstance->SaveCharacterStats.RMBRotationPurButtonCount = RMBRotationPurButtonCount;
		SaveGameInstance->SaveCharacterStats.FDamagePurButtonCount = FDamagePurButtonCount;

		//if (bSwitchLevel == false) //Level ��ȯ�� �ƴҶ��� map�̸��� �����Ѵ�.
		{
			//Level name����.
			FString MapName = GetWorld()->GetMapName();
			MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix); //PIE_0��¼�� ��¼�� �����Ѵ�.
			SaveGameInstance->SaveCharacterStats.MapName = MapName;
			UE_LOG(LogTemp, Warning, TEXT("MapName : %s"), *MapName);
		}

		

		if (EquippedWeapon) //������ ���Ⱑ ������
		{
			//������ ������ WEaponName�� ������.
			SaveGameInstance->SaveCharacterStats.SaveWeaponName = EquippedWeapon->WeaponName;
		}

		SaveGameInstance->SaveCharacterStats.Location = GetActorLocation();
		SaveGameInstance->SaveCharacterStats.Rotation = GetActorRotation();

		//SaveGame Slot�� �־��ش�.
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SlotName, SaveGameInstance->UserIndex);
	}
}

void AMainCharacter::LoadGame(bool bSwitchLevel)
{
	/*OpenLevel�� ���� �ʱ�ȭ �Ǳ⶧����  Save, Load�� ���������� ȣ������� Data�� �̾���.
	*
	* LevelTransition�� overlap�Ȱ� üũ�ؼ� üũ�� �Ǿ��ִٸ�
	* Load�ÿ� Location, Rotation�� �ε� ���ϴ°ɷ� �ϱ�.
	*/
	
	//Save���� ����������, USaveGameCustom Ŭ������ savegame object�� �����ϰ�
	//SaveGame���� �ʱ�ȭ �Ǿ��ִ� Name�� Index���� �̿��� Slot���ش��ϴ� ��ü�� �ҷ��´�.
	USaveGameCustom* SaveGame = Cast<USaveGameCustom>(UGameplayStatics::CreateSaveGameObject(USaveGameCustom::StaticClass()));

	USaveGameCustom* LoadGameInstance = Cast<USaveGameCustom>(UGameplayStatics::LoadGameFromSlot(SaveGame->SlotName, SaveGame->UserIndex));

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();

	if (LoadGameInstance)
	{
		if (AnimInst)//�װ� �� �� Load������ Movement Status�� Anim update�� �ٽ� Ǯ���ش�.
		{
			if (AnimInst->IsAnyMontagePlaying())
			{
				UAnimMontage* playingMontage = AnimInst->GetCurrentActiveMontage();
				if (playingMontage == DeathMontage)
				{
					UE_LOG(LogTemp, Warning, TEXT("Anim name is : %s"), *playingMontage->GetFName().ToString());
					AnimInst->Montage_Stop(0.f, playingMontage);
				}
			}
		
			SetMovementStatus(EMovementStatus::EMS_Normal);
			GetMesh()->bPauseAnims = false;
			GetMesh()->bNoSkeletonUpdate = false;
		}
		FallingMaxHeight = 0.f; //���� �������� ������ �ε��� ��� �������� �ִ� ���̸� �ʱ�ȭ �����ش�.

		Health = LoadGameInstance->SaveCharacterStats.Health;
		MaxHealth = LoadGameInstance->SaveCharacterStats.MaxHealth;
		Stamina = LoadGameInstance->SaveCharacterStats.Stamina;
		MaxStamina = LoadGameInstance->SaveCharacterStats.MaxStamina;
		Coins = LoadGameInstance->SaveCharacterStats.Coins;
		Souls = LoadGameInstance->SaveCharacterStats.Souls;
		PlayerDamage = LoadGameInstance->SaveCharacterStats.PlayerDamage;
		ThrowAbility_Distance = LoadGameInstance->SaveCharacterStats.ThrowAbility_Distance;
		ThrowAbility_Rotation = LoadGameInstance->SaveCharacterStats.ThrowAbility_Rotation;
		SmashAbility_Damage = LoadGameInstance->SaveCharacterStats.SmashAbility_Damage;


		HealthPurButtonCount = LoadGameInstance->SaveCharacterStats.HealthPurButtonCount;
		StaminaPurButtonCount = LoadGameInstance->SaveCharacterStats.StaminaPurButtonCount;
		DamagePurButtonCount = LoadGameInstance->SaveCharacterStats.DamagePurButtonCount;
		RMBDistancePurButtonCount = LoadGameInstance->SaveCharacterStats.RMBDistancePurButtonCount;
		RMBRotationPurButtonCount = LoadGameInstance->SaveCharacterStats.RMBRotationPurButtonCount;
		FDamagePurButtonCount = LoadGameInstance->SaveCharacterStats.FDamagePurButtonCount;

		
		


		if (WeaponSave)//AItemSave�� ������������ (�ȿ� Tmap����)
		{
			AItemSave* Weapon = GetWorld()->SpawnActor<AItemSave>(WeaponSave); //�ش� ��ü�� ���忡 �����ϰ�
			if (Weapon) //���������� �����ϸ�
			{
				FString WeaponName = LoadGameInstance->SaveCharacterStats.SaveWeaponName; //����� ������ �̸��� ��������
				if (WeaponName != TEXT(""))
				{
					AWeapon* EquipWeapon = GetWorld()->SpawnActor<AWeapon>(Weapon->WeaponData[WeaponName]); //�� �̸��� ���� AWeaponclass�� �����Ѵ�.
					if (EquipWeapon)
					{
						EquipWeapon->SetWeaponOwner(this);
						EquipWeapon->Equip(this);
					}
				}
			}

		}
		
		if (bSwitchLevel == false) //Transition volume�� ���� �ʾҴٸ� Location�� �ҷ��� �����Ѵ�.
		{
			SetActorLocation(LoadGameInstance->SaveCharacterStats.Location);
			SetActorRotation(LoadGameInstance->SaveCharacterStats.Rotation);
		}
		else
		{
			bSwitchLevel = false;
		}
	}
	
}

void AMainCharacter::LoadGame_FirstLoad()
{
	USaveGameCustom* SaveGame = Cast<USaveGameCustom>(UGameplayStatics::CreateSaveGameObject(USaveGameCustom::StaticClass()));

	USaveGameCustom* LoadGameInstance = Cast<USaveGameCustom>(UGameplayStatics::LoadGameFromSlot(SaveGame->SlotName, SaveGame->UserIndex));

	if (LoadGameInstance)
	{
		//�ʺ��� �ҷ��ͼ� ��Ī��Ų��. �ٸ��ʿ��� ����ȸ��� �ε�ÿ� ������ Open Level�� ȣ���ϸ鼭 �ؿ��� �� ���ư�.
		if (LoadGameInstance->SaveCharacterStats.MapName != TEXT(""))
		{
			FString CurrentMapName = GetWorld()->GetMapName();
			CurrentMapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
			if (LoadGameInstance->SaveCharacterStats.MapName == TEXT("Firstmap"))
			{
				LoadGame(false);
			}
			else if (CurrentMapName != LoadGameInstance->SaveCharacterStats.MapName) //����� �� name�� ���� map name�� �ٸ���
			{
				FName OpenMapName = FName(*LoadGameInstance->SaveCharacterStats.MapName); //����� ���� �����Ѵ�.

				LoadGameInstance->SaveCharacterStats.bFirstLoadGame = true;
				UGameplayStatics::SaveGameToSlot(LoadGameInstance, LoadGameInstance->SlotName, LoadGameInstance->UserIndex);

				UGameplayStatics::OpenLevel(this, OpenMapName);
			}
		}
	}
}