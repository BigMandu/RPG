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
	class UAIPerceptionStimuliSourceComponent* StimuliSourceComponent; //������Ʈ ����,
	TSubclassOf<class UAISense_Sight> SenseSight; //Sight Sense�� ����ϱ����ؼ� �ʿ�.


	//////////// ī�޶� ///////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;
	
	/* ī�޶� ȸ�� ���� */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;


	//Capsule Component �浹 ����

	bool bCapsuleHit;

	UFUNCTION()
	void CapsuleOnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/*****************************/
	// Player Input   //
	/*****************************/
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

	/*****************************/
	// Player Stats   //
	/*****************************/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats");
	float MaxHealth;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats");
	float Health;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats");
	float MaxStamina;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats");
	float Stamina;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats");
	float PlayerDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats");
	int32 Coins;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats");
	int32 Souls;

	/*******************************/
	//-- Player Movement   ---//
	/*******************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* PlayerController;

	//������ Enum
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;
	
	//Stamina Enum
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintingSpeed;

	//Sprint�� Stamina�� ���ҵǴ� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	//Stamina�� ȸ���Ǵ� ����.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaRecoveryRate;

	//Stamina�� �ּ� ���ؼ�, �پ��� ȸ���ɶ� �� ������ ���� ���ϸ�, Stamina�Ҹ� ��� �ߵ� �Ұ�.
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

	FTimerHandle SprintAttackTimer; //SprintAttack�� ���Ǵ� Ÿ�̸��ڵ�
	FTimerHandle AbilitySmashHandle; // Ability Smash�� ���Ǵ� Ÿ�̸� �ڵ�

	float CurHeight;

	FTimerHandle FallingTimer; //���� ������ Ÿ�̸�

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
	//AController* Instigator; //ApplyDamage�� �Ѱ��� ��Ʈ�ѷ�

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

	//// ȸ�� ////
	//@param Rate ����ȭ ����. 1.0 == ���ϴ� ȸ���� 100%
	void TurnAtRate(float Rate);
	void LookUpRate(float Rate);

	/*****************************/
	// Player Input �Լ�  //
	/*****************************/
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
	//-- Player Movement �Լ� ---//
	/*******************************/
	void MoveForward(float Value);
	void MoveRight(float Value);

	void CharacterRotate();

	void SetMovementStatus(EMovementStatus Status);
	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }


	////Coin, Soul function////
	void IncrementCoin(int32 Amount);
	//void DecrementCoin(int32 Amount);
	void IncrementSoul(int32 Amount);
	//void DecrementSoul(int32 Amount);

	void IncrementHealth(float Amount);

	/*******************************/
	//---     Player Weapon     ---//
	/*******************************/
	void SetEquippedWeapon(class AWeapon* WeaponToSet);
	FORCEINLINE void SetActiveOverlappingItem(class AItem* ItemToSet) { OverlappingItem = ItemToSet; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	AItem* OverlappingItem;

	/*******************************/
	//---     Player Combat     ---//
	/*******************************/

	// Enemy Bluepirnt�� ����. EnemyAIController���� Enemy�� �÷��̾ �߽߰� �ش� �Լ� ȣ����.
	/*class AEnemy* HasSpotted;
	FORCEINLINE void SetEnemyFindPlayer(class AEnemy* WhoSpotted) { HasSpotted = WhoSpotted; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AEnemy* GetEnemyFindPlayer() { return HasSpotted; }*/
	
	void DecrementHealth(float Amount);
	void Die();

	void AttackGiveDamage(class AEnemy* DamagedEnemy, float WeaponDamage);

	void AttackRangeDamage();

	void FallingDamageCalc();

	void TakeFallingDamage(float AfterHeight);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Attack();

	void AttackAir();

	void SptirntAttack();

	bool CalcAirAttack();

	void Ability_ThrowWeapon();
	void Ability_ThrowWeapon_Finish();

	void Ability_Smash();
	void Ability_Smash_Finish();

	UFUNCTION(BlueprintCallable)
	void ComboSave();

	UFUNCTION(BlueprintCallable)
	void ComboReset();

	



	/*******************************/
	//--- Game Level,Save,Load  ---//
	/*******************************/
	//Level TransitionVolume�� Overlap�ɶ� ȣ��� �Լ�.
	void SwitchLevel(FName LevelName);

};

