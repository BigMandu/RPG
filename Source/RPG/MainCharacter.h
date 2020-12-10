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

	/*****************************/
	// Player Input   //
	/*****************************/
	bool bShiftKeyDown;
	bool bEKeyDown;
	bool bLMBDown;
	bool bJumpKeyDown;

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

	FTimerHandle SprintAttackTimer; //SprintAttack시 사용되는 타이머핸들

	float CurHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bSaveAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 AttackCount;

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
	void ShiftKeyDown();
	void ShiftKeyUp();
	void EKeyDown();
	void EKeyUp();
	void LMBDown();
	void LMBUp();
	virtual void Jump() override;
	virtual void StopJumping() override;

	/*******************************/
	//-- Player Movement 함수 ---//
	/*******************************/
	void MoveForward(float Value);
	void MoveRight(float Value);

	void CharacterRotate();

	void SetMovementStatus(EMovementStatus Status);
	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	////Damage관련 함수////
	void DecrementHealth(float Amount);
	void Die();

	////Coin, Soul function////
	void IncrementCoin(int32 Amount);
	//void DecrementCoin(int32 Amount);
	//void IncrementSoul(int32 Amount);
	//void DecrementSoul(int32 Amount);

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
	void AttackDamage(class AEnemy* DamagedEnemy, float WeaponDamage);

	void Attack();

	void AttackAir();

	void SptirntAttack();

	bool CalcAirAttack();

	UFUNCTION(BlueprintCallable)
	void ComboSave();

	UFUNCTION(BlueprintCallable)
	void ComboReset();

	



	/*******************************/
	//--- Game Level,Save,Load  ---//
	/*******************************/
	//Level TransitionVolume에 Overlap될때 호출될 함수.
	void SwitchLevel(FName LevelName);

};

