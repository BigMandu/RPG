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
	bMoveForward = false;
	bMoveRight = false;

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
			FallingDamageCalc();
		}
		else if (GetCharacterMovement()->IsFalling() == false)
		{
			AfterHeight = OutHit.ImpactPoint.Z;
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
///////// Coin, Soul 관련 함수 /////
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
	FCollisionQueryParams Params(FName(TEXT("PlayerRangeDamage")), false, this);
	TArray<FHitResult>OutHit;
	FVector StartLocation = GetActorLocation(); //StartLocation
	FVector WeaponLength = EquippedWeapon->CombatCollision->GetScaledBoxExtent() * 2.0f;

	FVector EndLocation = GetActorForwardVector() * WeaponLength.Z + StartLocation;

	//Weapon의 CombatCollision * 2 크기를 가진 캡슐모양으로 판정한다.
	GetWorld()->SweepMultiByChannel(OutHit, StartLocation, EndLocation, FQuat::Identity, 
		ECollisionChannel::ECC_Pawn, FCollisionShape::MakeCapsule(WeaponLength), Params);
	
	float Damage = PlayerDamage;
	bool AirAttack = false;
	if (GetCharacterMovement()->IsFalling()) //떨어지고 있으면, 낙하높이에 15%의 데미지를 더 줌.
	{
		AirAttack = true;
		Damage += CurHeight * 0.15f;
		//UE_LOG(LogTemp, Warning, TEXT("Damage is : %f, Weapon Damage is : %f, TotalDamage is : %f"), Damage, EquippedWeapon->WeaponDamage, Damage + EquippedWeapon->WeaponDamage);
	}

	float ZSize = WeaponLength.Z;
	float YSize = WeaponLength.Y;
	float XSize = WeaponLength.X;

	//디버깅용
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
			//디버깅용
			{
				DrawDebugCapsule(GetWorld(), EndLocation,
					ZSize * 0.5 + XSize + YSize, XSize + YSize, FRotationMatrix::MakeFromZ(GetActorForwardVector() * ZSize).ToQuat(),
					FColor::Green, false, 2.0f);
				//UE_LOG(LogTemp, Warning, TEXT("Player Range Attack success, Damage is : %f, Weapon Damage is : %f, TotalDamage is : %f"), Damage, EquippedWeapon->WeaponDamage, Damage + EquippedWeapon->WeaponDamage);
			}
			if (AirAttack == true)
			{
				//Air Attack이면 땅에 떨어지고 난뒤에 Damage적용 하기.
			}
			UGameplayStatics::ApplyDamage(Enemy, Damage + EquippedWeapon->WeaponDamage, GetController(), this, DamageTypeClass);
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
	
	 // UKismetMathLibrary::Abs(FallingMaxHeight - AfterHeight)
	if (FallingMaxHeight-AfterHeight >= GetDefaultHalfHeight() * 3.f) //자기키의  1.5배가 되면 낙하데미지를 받는다. 
	{
		FallingDamage = FallingMaxHeight * 0.05f; //높이에서 5%를 데미지로 준다.
		DecrementHealth(FallingDamage);
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
					
					AddMovementInput(TargetVector, 1.0f * GetVelocity().Size());//캐릭터 속도 비율에 따라 앞으로 움직임.
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