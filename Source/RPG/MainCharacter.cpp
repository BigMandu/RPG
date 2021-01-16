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

	//Load할때 FirstMap을 거쳐서 로딩이 되기때문에 이를 체크함.
	bFirstLoad = false;

	//Smash Ability사용중 어딘가에 부딪히면 바로 멈추게 하기 위해서.
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

	bClickRMB = false;

	/*******************************/
	//-- Player Movement  ---//
	/*******************************/

	bJumped = false;

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
	StimuliSourceComponent->RegisterForSense(SenseSight); //Sight Sense를 등록.


	//캡슐컴포넌트의 힛 판정 함수
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AMainCharacter::CapsuleOnHit);


	//Abiltiy cooldown을 위함.
	ThrowTick = ThrowWeaponCooldown;
	SmashTick = SmashCooldown;

	bCanThrow = true;
	bCanSmash = true;
	///Beginplay에 초기화를 해줘서 level transition이나, 로드할때 바로 사용할 수 있도록 해준다.

	/// 맵로딩을 위한 작업
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
				SaveGame(true); //SaveGame의 bool값은 이제 의미 없음.
			}
			else
			{
				LoadGame(true); //장착된 무기, 스텟들을 로드한다.
				SaveGame(true); //그리고 다시 저장한다.
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

	//죽은 상태면 그냥 리턴.
	if (MovementStatus == EMovementStatus::EMS_Dead)
		return;

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
		FVector EndPoint = FVector(StartPoint.X, StartPoint.Y, -990.f);

		FCollisionQueryParams CollisionParams;
		//DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 1.f, 0, 2); //Line Trace 시각화 Debug

		bool isHit = GetWorld()->LineTraceSingleByChannel(OutHit, StartPoint, EndPoint, ECollisionChannel::ECC_Visibility, CollisionParams);

		if (isHit)
		{
			if (OutHit.bBlockingHit)
			{
				CurHeight = StartPoint.Z - OutHit.ImpactPoint.Z;
				//디버기용
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
			FallingDamageCalc(StartPoint.Z); //낙하시 최대 높이를 구하는함수.
			bJumped = true;
		}
		else if (GetCharacterMovement()->IsFalling() == false && bJumped == true)
		{
			//AfterHeight = OutHit.ImpactPoint.Z; //착지한 현재 높이를 넘겨준다.
			AfterHeight = OutHit.ImpactPoint.Z;
			TakeFallingDamage(AfterHeight);
			bJumped = false;
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
//////////   Input 관련 함수  ////////
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

//CapsuleComponent 충돌 관련
void AMainCharacter::CapsuleOnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Hit.bBlockingHit && bAttacking)
	{
		bCapsuleHit = true;
		//UE_LOG(LogTemp, Warning, TEXT("Capsule hit! %s"), *(Hit.Actor->GetFName().ToString()));
	}
}


/*************** Movement **************/
//////////   Movement 관련 함수  ////////
/**************************************/

void AMainCharacter::MoveForward(float Value)
{
	bMoveForward = false;
	if ((PlayerController != nullptr) && (Value != 0.f) && (!bAttacking) && MovementStatus != EMovementStatus::EMS_Dead)
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
					ECollisionChannel::ECC_WorldStatic); //마지막 인자값은, 타격 원점과 액터사이에 해당 채널을 막는 액터가 있다면 damamge를 받지 않는다는것임.

				//디버깅용
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
				//디버깅용
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

void AMainCharacter::FallingDamageCalc(float JumpHeight) //낙하시 최대 높이를 구한다.
{
	if (FallingMaxHeight < JumpHeight)
	{
		FallingMaxHeight = JumpHeight; //feet location.z
	}
}

void AMainCharacter::TakeFallingDamage(float AfterHeight) //outhit impact.z
{
	//떨어지고 착지 이후에 데미지 계산, 적용 및 초기화를 해준다.
	
	if (bAbilitySmash == true) //해당 ability가 재생중(실행중)이면 낙하뎀지 무시.
	{
		FallingMaxHeight = -999.f;
		return;
	}
	//떨어질때의 최대높이와 착지후 현재높이의 차이가 자기키의 1.5배가 넘으면 데미지를 받게 했다. -> 높은곳에서 높은곳으로 점프할때 데미지 받는걸 방지하기 위해서.
	if ( FallingMaxHeight-AfterHeight >= GetDefaultHalfHeight() * 3.f) //자기키의  1.5배가 되면 낙하데미지를 받는다. 
	{
		FallingDamage = (FallingMaxHeight - AfterHeight) * 0.05f; //떨어진 높이에서 5%를 데미지로 준다.
		DecrementHealth(FallingDamage);
		//디버깅용
		/*UE_LOG(LogTemp, Warning, TEXT("FMH is : %f, AfterHeight is : %f"), FallingMaxHeight, AfterHeight);
		UE_LOG(LogTemp, Warning, TEXT("Subtract is : %f"), FallingMaxHeight - AfterHeight);
		UE_LOG(LogTemp, Warning, TEXT("Falling Damage is : %f"), FallingDamage);*/
	}
	/*UE_LOG(LogTemp, Warning, TEXT("FMH is : %f, AfterHeight is : %f"), FallingMaxHeight, AfterHeight);
	UE_LOG(LogTemp, Warning, TEXT("Subtract is : %f"), FallingMaxHeight - AfterHeight);*/
	//관련 변수 초기화.
	FallingDamage = 0.f;
	FallingMaxHeight = -999.f;

}

float AMainCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	//UE_LOG(LogTemp, Warning, TEXT("MainCharacter::TakeDamage()::::Damage Causer : %s , Damage : %f"), *DamageCauser->GetName(), DamageAmount);
	
	if (bAbilitySmash == false) //해당 어빌리티 사용시 Damage를 받지 않기 하기 위함.
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

void AMainCharacter::Ability_ThrowWeapon_Finish() //AWeapon::ReceiveWeapon에서 호출
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AbilityThrowWeaponMontage)
	{
		AnimInstance->Montage_Play(AbilityThrowWeaponMontage, 1.5f);
		AnimInstance->Montage_JumpToSection(FName("Finish"), AbilityThrowWeaponMontage);

		//끝난 후 부터 쿨타임 생성.
		GetWorldTimerManager().SetTimer(ThrowWeaponCooldownHandle, this, &AMainCharacter::Ability_ThrowWeapon_Cooldown, GetWorld()->GetDeltaSeconds(), true);
	}
}

void AMainCharacter::Ability_Smash()
{
	//if (EquippedWeapon && AbilitySmashMontage && !bAttacking && !GetCharacterMovement()->IsFalling()) //Cooldown check에서 해버린다.
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

			AnimInstance->Montage_Play(AbilitySmashMontage, 1.4f);
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


	//끝난후 부터 쿨타임을 돌린다.
	GetWorldTimerManager().SetTimer(SmashCooldownHandle, this, &AMainCharacter::Ability_Smash_Cooldown, GetWorld()->GetDeltaSeconds(), true);
}


//**************************************************//
//				Ability Cool down 적용				//
//**************************************************//

bool AMainCharacter::Ability_ThrowWeapon_Cooldown_Check()
{
	//ThrowWeaponCooldown = 3.f; 초기값
	if (EquippedWeapon && !bAttacking) //여기서 검증을 한다.
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

void AMainCharacter::Ability_ThrowWeapon_Cooldown() //Ability_ThrowWeapon_Finish에서 Timer로 호출한다.
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
	//SmashCooldown = 5.f; 초기값

	//여기서 아예 검증을 해버린다
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



void AMainCharacter::Ability_Smash_Cooldown() //Ability_Smash_Finish에서 Timer로 호출한다.
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
void AMainCharacter::SwitchLevel(FName LevelName) //Level Transition Volume에 오버랩되면 호출됨
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString MapName = World->GetMapName();
		FName CurrentLevelName(*MapName);

		//변경할 Level과 현재Level이 다를때 Open world를 한다.
		if (CurrentLevelName != LevelName)
		{
			SaveGame(true); //레벨 전환시에 저장을 해줘야 무기를 갖고온다.

			UGameplayStatics::OpenLevel(World, LevelName); //Open level하면 beginplay가 실행됨.
		}
	}
}

void AMainCharacter::SaveGame(bool bSwitchLevel)
{
	/*CreateSaveGameObject은 SaveGame class를 인자로 받고, SaveGame을 리턴한다.
	* 따라서 그냥 USaveGameCustom* Savegame = Cast<USaveGameCustom>(ㅇㅇ) 이렇게 해도 되지만 
	* 가독성을 위해 아래 처럼 해봤다.
	* 
	* USaveGameCustom class를 갖고 SaveGameObject를 생성한것을 SaveGame에 지정이됐고
	* USaveGameCustom의 참조변수에 위에서 생성한 class를 캐스트해서 넣어줬다.
	*/
	
	USaveGame* SaveGame = UGameplayStatics::CreateSaveGameObject(USaveGameCustom::StaticClass());
	USaveGameCustom* SaveGameInstance = Cast<USaveGameCustom>(SaveGame);
	if (SaveGameInstance) //이 생성된 객체안에 있는 구조체에 저장할 변수들을 저장한다.
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

		//if (bSwitchLevel == false) //Level 전환이 아닐때만 map이름을 저장한다.
		{
			//Level name저장.
			FString MapName = GetWorld()->GetMapName();
			MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix); //PIE_0어쩌고 저쩌고를 삭제한다.
			SaveGameInstance->SaveCharacterStats.MapName = MapName;
			UE_LOG(LogTemp, Warning, TEXT("MapName : %s"), *MapName);
		}

		

		if (EquippedWeapon) //장착된 무기가 있으면
		{
			//장착된 무기의 WEaponName을 저장함.
			SaveGameInstance->SaveCharacterStats.SaveWeaponName = EquippedWeapon->WeaponName;
		}

		SaveGameInstance->SaveCharacterStats.Location = GetActorLocation();
		SaveGameInstance->SaveCharacterStats.Rotation = GetActorRotation();

		//SaveGame Slot에 넣어준다.
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SlotName, SaveGameInstance->UserIndex);
	}
}

void AMainCharacter::LoadGame(bool bSwitchLevel)
{
	/*OpenLevel시 모든게 초기화 되기때문에  Save, Load를 순차적으로 호출해줘야 Data가 이어짐.
	*
	* LevelTransition에 overlap된걸 체크해서 체크가 되어있다면
	* Load시에 Location, Rotation은 로드 안하는걸로 하기.
	*/
	
	//Save때와 마찬가지로, USaveGameCustom 클래스로 savegame object를 생성하고
	//SaveGame내에 초기화 되어있는 Name과 Index값을 이용해 Slot에해당하는 객체를 불러온다.
	USaveGameCustom* SaveGame = Cast<USaveGameCustom>(UGameplayStatics::CreateSaveGameObject(USaveGameCustom::StaticClass()));

	USaveGameCustom* LoadGameInstance = Cast<USaveGameCustom>(UGameplayStatics::LoadGameFromSlot(SaveGame->SlotName, SaveGame->UserIndex));

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();

	if (LoadGameInstance)
	{
		if (AnimInst)//죽고 난 뒤 Load했을때 Movement Status와 Anim update를 다시 풀어준다.
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
		FallingMaxHeight = 0.f; //높이 떨어지고 있을때 로드할 경우 떨어지는 최대 높이를 초기화 시켜준다.

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

		
		


		if (WeaponSave)//AItemSave를 지정해줬으면 (안에 Tmap있음)
		{
			AItemSave* Weapon = GetWorld()->SpawnActor<AItemSave>(WeaponSave); //해당 객체를 월드에 스폰하고
			if (Weapon) //성공적으로 스폰하면
			{
				FString WeaponName = LoadGameInstance->SaveCharacterStats.SaveWeaponName; //저장된 무기의 이름을 가져오고
				if (WeaponName != TEXT(""))
				{
					AWeapon* EquipWeapon = GetWorld()->SpawnActor<AWeapon>(Weapon->WeaponData[WeaponName]); //그 이름을 갖고 AWeaponclass를 스폰한다.
					if (EquipWeapon)
					{
						EquipWeapon->SetWeaponOwner(this);
						EquipWeapon->Equip(this);
					}
				}
			}

		}
		
		if (bSwitchLevel == false) //Transition volume에 들어가지 않았다면 Location을 불러와 세팅한다.
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
		//맵부터 불러와서 매칭시킨다. 다른맵에서 저장된맵을 로드시에 어차피 Open Level을 호출하면서 밑에껀 다 날아감.
		if (LoadGameInstance->SaveCharacterStats.MapName != TEXT(""))
		{
			FString CurrentMapName = GetWorld()->GetMapName();
			CurrentMapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
			if (LoadGameInstance->SaveCharacterStats.MapName == TEXT("Firstmap"))
			{
				LoadGame(false);
			}
			else if (CurrentMapName != LoadGameInstance->SaveCharacterStats.MapName) //저장된 맵 name과 현재 map name이 다르면
			{
				FName OpenMapName = FName(*LoadGameInstance->SaveCharacterStats.MapName); //저장된 맵을 오픈한다.

				LoadGameInstance->SaveCharacterStats.bFirstLoadGame = true;
				UGameplayStatics::SaveGameToSlot(LoadGameInstance, LoadGameInstance->SlotName, LoadGameInstance->UserIndex);

				UGameplayStatics::OpenLevel(this, OpenMapName);
			}
		}
	}
}