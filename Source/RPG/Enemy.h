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
enum class EEnemyType : uint8
{
	ET_Creature		UMETA(DisplayName = "Creature"),
	ET_Humanoid		UMETA(DisplayName = "Humanoid"),
	ET_Boss			UMETA(DisplayName = "Boss"),

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
	///////////////////////////////


	//////////   Enemy Type   ///////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EEnemyType EnemyType;

	FORCEINLINE EEnemyType GetEnemyType() { return EnemyType; }

	bool EnemyisAlive();


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



	//////////////////////////////
	/****    Enemy Combat    ****/
	//////////////////////////////
	
	class UEnemyAnimInstance* AnimInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* CloseCombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* DashAttackCombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* SpiderHitDeathMontage;

	UFUNCTION()
	void AttackGiveDamage();

	bool ReturnHit();
	bool bWasHit;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();
	
	void DeathClear();

	UFUNCTION()
	void Attack(UBlackboardComponent* BBComp);

	//UFUNCTION()
	//void OnCombatMontageEnded(UAnimMontage* Montage, bool bInterrupted); //OnMontageEnded delegate와 연결할 함수.

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

	/*
	//////////////AI TEST////////////////////
	class UNavigationSystemV1* NavSystem; //TEST목적, RandomLocation을 얻는 함수를 사용하기 위해.
	void MoveToRandomLocation();

	
	FTimerHandle LostTimer;
	FTimerDelegate LostDelegate;
	class UAIPerceptionComponent* PerceptionComponent;
	class UAISenseConfig_Sight* SenseSightConfig;

	UFUNCTION()
	void DetectActor(AActor* Actor, FAIStimulus Stimulus);
	void TargetLost(AActor* Actor);
	*/

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
