// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionTypes.h"
#include "Enemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyMovementStatus : uint8
{
	EMS_Patrol		UMETA(DisplayName = "Patrol"),
	EMS_Search		UMETA(DisplayName = "Search"),
	EMS_Chase		UMETA(DisplayName = "Chase"),
	EMS_Attack		UMETA(DisplayName = "Attack"),
	EMS_Dead		UMETA(DisplayName = "Dead"),

	EMS_MAX			UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EEnemyWeaponType : uint8
{
	EWT_Weapon		UMETA(DisplayName = "HasWeapon"),
	EWT_NoWeapon	UMETA(DisplayName = "NoWeapon"),

	ET_MAX			UMETA(DisplayName = "DefaultMAX")
};



UCLASS()
class RPG_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	///////////////////////////////
	/**     EnemyMovement    ****/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EEnemyMovementStatus EnemyMovementStatus;

	FORCEINLINE void SetEnemyMovementStatus(EEnemyMovementStatus EnemyStatus) { EnemyMovementStatus = EnemyStatus; }

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float CurrentMovementSpeed;
	///////////////////////////////


	//////////   Enemy Type   ///////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EEnemyWeaponType EnemyWeaponType;

	FORCEINLINE EEnemyWeaponType GetEnemyType() { return EnemyWeaponType; }

	class AMainCharacter* MainPlayer;




	//bool EnemyisAlive();

	///////////////////////////
	//Effect
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Particles")
	class UParticleSystem* HitParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	class USoundCue* HitSound;


	//////////////////////////////
	/****      Enemy UI      ****/
	//////////////////////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> WEnemyHealthBar;
		//class UUserWidget* WEnemyHealthBar;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	class UWidgetComponent* EnemyHPbarComp;

	class UEnemyWidget* EnemyWidget;

	//////////////////////////////
	/****      Enemy Stats   ****/
	//////////////////////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRadius;


	//////////////////////////////
	/****      Enemy AI      ****/
	//////////////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AEnemyAIController* AIController; //AI Controller클래스

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class USphereComponent* AgroSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	USphereComponent* CombatSphere;

	UPROPERTY(EditAnywhere, Category = "Behavior")
	class UBehaviorTree* EnemyBehavior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolArea; //정찰 범위 BTTask_SearchPatrolLocation에서 사용함.


	//Enemy가 죽고난뒤

	UPROPERTY(EditAnywhere, Category = "Loot")
	int32 SoulMin;
	UPROPERTY(EditAnywhere, Category = "Loot")
	int32 SoulMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FVector SpawnItemArea;

	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	TSubclassOf<class ASoul> Soul;
	//class ASoul* SoulClass;
	void SpawnLoot();


	//////////////////////////////
	/****    Enemy Combat    ****/
	//////////////////////////////
	
	//Enemy의 Weapon
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AWeapon> EnemyWeapon;

	//Collision을 각자 활성화/비활성화를 위함.
	AWeapon* LeftWeapon;
	AWeapon* RightWeapon;

	void ActivateCollision();
	void DeactivateCollision();
	
	class UEnemyAnimInstance* AnimInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* CloseCombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* DashAttackCombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* HitDeathMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasRangeAttack;

	UFUNCTION()
	void AttackRangeDamage();

	void AttackGiveDamage(ACharacter* Victim);

	bool bStrongAttack;

	bool ReturnHit();
	bool bWasHit;

	void DecrementalHealth(float Damage);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();
	
	void DeathClear();

	UFUNCTION()
	void Attack(UBlackboardComponent* BBComp);

	UFUNCTION()
	void AttackEnd();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking;

	FTimerHandle DashAttackHandle; //DashAttack때 사용할 TimerHandle

	TSubclassOf<UDamageType>DamageTypeClass;
	
	
	UFUNCTION()
	void RotateToTarget();//Target(Player)를 향해 회전. AIController에서 호출.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 NumberOfCombatAnim; //Animation개수를 넣어줌. defaults value = 3;

	int32 Section; //특정 Animation을 재생하기 위한 값.



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void AgroSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void AgroSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	virtual void CombatSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void CombatSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
