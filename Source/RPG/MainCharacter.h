// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprint UMETA(DisplayName = "Sprint"),
	EMS_Dead	UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	ESS_Normal			UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum	UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted		UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovery UMETA(DisplayName = "ExhaustedRecovery"),
	ESS_Recovery		UMETA(DisplayName = "Recovery"),

	ESS_MAX UMETA(DisplayName = "DefaultMAX")
};



UCLASS()
class RPG_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter();


	/////////////////AI TEST ////////////////
	class UAIPerceptionStimuliSourceComponent* StimuliSourceComponent; //컴포넌트 생성,
	TSubclassOf<class UAISense_Sight> SenseSight; //Sight Sense를 등록하기위해서 필요.


	//////////// 카메라 ///////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;
	
	/* 카메라 회전 관련 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Sound")
	class USoundCue* LevelFrozenCaveSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Sound")
	USoundCue* LevelDungeonSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	class USoundCue* HitSound;

	class UAudioComponent* LevelAudioComponent;

	//Capsule Component 충돌 관련

	bool bCapsuleHit;

	UFUNCTION()
	void CapsuleOnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/*****************************/
	// Player Input   //
	/*****************************/
	bool bESCKeyDown;
	bool bShiftKeyDown;
	bool bEKeyDown;
	bool bLMBDown;
	bool bRMBDown;
	bool bJumpKeyDown;
	bool bFkeyDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bMoveForward;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bMoveRight;


	bool bClickRMB;

	/*****************************/
	// Player Stats   //
	/*****************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats");
	float MaxHealth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats");
	float Health;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats");
	float MaxStamina;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats");
	float Stamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats");
	float PlayerDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats");
	int32 Coins;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats");
	int32 Souls;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats");
	float ThrowAbility_Distance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats");
	float ThrowAbility_Rotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats");
	float SmashAbility_Damage;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store");
	int32 HealthPurButtonCount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store");
	int32 StaminaPurButtonCount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store");
	int32 DamagePurButtonCount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store");
	int32 RMBDistancePurButtonCount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store");
	int32 RMBRotationPurButtonCount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store");
	int32 FDamagePurButtonCount;


	/*******************************/
	//-- Player Movement   ---//
	/*******************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* PlayerController;

	//움직임 Enum
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;
	
	//Stamina Enum
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintingSpeed;

	//Sprint시 Stamina가 감소되는 비율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	//Stamina가 회복되는 비율.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaRecoveryRate;

	//Stamina의 최소 기준선, 다쓰고 회복될때 이 기준을 넘지 못하면, Stamina소모 기술 발동 불가.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinStamina; 


	/*******************************/
	//---     Player Combat     ---//
	/*******************************/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	class UAnimMontage* CombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* AirCombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* SprintCombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* AbilityThrowWeaponMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* AbilitySmashMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* DeathMontage;

	FTimerHandle SprintAttackTimer; //SprintAttack시 사용되는 타이머핸들
	FTimerHandle AbilitySmashHandle; // Ability Smash때 사용되는 타이머 핸들

	FTimerHandle RMBDownTimerHandle; //Ability Throw REady때 사용됨.

	float CurHeight;

	FTimerHandle FallingTimer; //낙하 데미지 타이머

	float FallingDamage;
	float FallingMaxHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bSaveAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 AttackCount;

	bool bAbilitySmash;
	

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	//AController* Instigator; //ApplyDamage에 넘겨줄 컨트롤러

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//// 회전 ////
	//@param Rate 정규화 비율. 1.0 == 원하는 회전율 100%
	void TurnAtRate(float Rate);
	void LookUpRate(float Rate);

	/*****************************/
	// Player Input 함수  //
	/*****************************/
	void ESCKeyDown();
	void ESCKeyUp();
	void ShiftKeyDown();
	void ShiftKeyUp();
	void EKeyDown();
	void EKeyUp();
	void LMBDown();
	void LMBUp();
	void RMBDown();
	void RMBUp();
	virtual void Jump() override;
	virtual void StopJumping() override;

	void FKeyDown();
	void FKeyUp();

	/*******************************/
	//-- Setting Player Stats 함수 ---//
	/*******************************/

	UFUNCTION(BlueprintCallable)
	void SetMaxHealthPoint(float MaxHP);

	UFUNCTION(BlueprintCallable)
	void SetMaxStamina(float MaxStat);

	UFUNCTION(BlueprintCallable)
	void SetPlayerDamage(float CharDamage);

	UFUNCTION(BlueprintCallable)
	void SetRMBDistance(float RMBDistance);

	UFUNCTION(BlueprintCallable)
	void SetRMBRotation(float RMBRotation);

	UFUNCTION(BlueprintCallable)
	void SetFDamage(float FDamage);

	/*******************************/
	//-- Player Movement 함수 ---// 
	/*******************************/
	void MoveForward(float Value);
	void MoveRight(float Value);

	void CharacterRotate();

	void SetMovementStatus(EMovementStatus Status);
	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	UFUNCTION(BlueprintCallable)
	EMovementStatus GetMovementStatus() { return MovementStatus; }


	////Coin, Soul function////
	void IncrementCoin(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void DecrementCoin(int32 Amount);
	
	void IncrementSoul(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void DecrementSoul(int32 Amount);

	/*******************************/
	//---     Player Weapon     ---//
	/*******************************/
	void SetEquippedWeapon(class AWeapon* WeaponToSet);
	FORCEINLINE void SetActiveOverlappingActor(class AActor* OverlapActor) { OverlappingActor = OverlapActor; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	AActor* OverlappingActor;

	/*******************************/
	//---     Player Combat     ---//
	/*******************************/

	bool bJumped;
	void FallingDamageCalc(float JumpHeight);

	void TakeFallingDamage(float AfterHeight);

	// Enemy Bluepirnt를 위함. EnemyAIController에서 Enemy가 플레이어를 발견시 해당 함수 호출함.
	/*class AEnemy* HasSpotted;
	FORCEINLINE void SetEnemyFindPlayer(class AEnemy* WhoSpotted) { HasSpotted = WhoSpotted; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AEnemy* GetEnemyFindPlayer() { return HasSpotted; }*/
	
	void IncrementHealth(float Amount);

	void DecrementHealth(float Amount);
	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	void AttackGiveDamage(class AEnemy* DamagedEnemy, float WeaponDamage);

	void AttackRangeDamage();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Attack();

	void AttackAir();

	void SptirntAttack();

	bool CalcAirAttack();

	UFUNCTION(BlueprintCallable)
	void ComboSave();

	UFUNCTION(BlueprintCallable)
	void ComboReset();

	/*******************************/
	//---     Player Ability     ---//
	/*******************************/

	void Ability_ThrowWeapon_Before();
	void Ability_ThrowWeapon();
	void Ability_ThrowWeapon_Finish();

	void Ability_Smash();
	void Ability_Smash_Finish();

	FTimerHandle ThrowWeaponCooldownHandle;
	FTimerHandle SmashCooldownHandle;

	bool bCanThrow;
	bool bCanSmash;

	bool Ability_ThrowWeapon_Cooldown_Check();
	bool Ability_Smash_Cooldown_Check();

	void Ability_ThrowWeapon_Cooldown();
	void Ability_Smash_Cooldown();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	float ThrowWeaponCooldown; //스킬 쿨타임
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	float SmashCooldown; //스킬 쿨타임

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float ThrowTick; //skill사용 직후 0부터 SmashCooldown까지 매 tick마다 + 되는 값.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float SmashTick; //skill사용 직후 0부터 SmashCooldown까지 매 tick마다 + 되는 값.

	/*******************************/
	//--- Game Level,Save,Load  ---//
	/*******************************/

	UFUNCTION(BlueprintCallable)
	void SwitchLevel(FName LevelName);

	UFUNCTION(BlueprintCallable)
	void SaveGame(bool bSwitchLevel);

	UFUNCTION(BlueprintCallable)
	void LoadGame(bool bSwitchLevel);

	UFUNCTION(BlueprintCallable)
	void LoadGame_FirstLoad();

	bool bFirstLoad;

	UPROPERTY(EditDefaultsOnly, Category = "SaveGameData")
	TSubclassOf<class AItemSave> WeaponSave;
};

