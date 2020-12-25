// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

//무기 상태
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Spawn		UMETA(DisplayName = "Spawn"), //world에 pickup상태로 spawn된 상태
	EWS_Equipped	UMETA(DisplayName = "Equipped"), //E key를 눌러서 장착한 상태
	EWS_Sheath		UMETA(DisplayName = "Sheath"), //장착 상태에서 비무장 상태
	EWS_Draw		UMETA(DisplayName = "Draw"), //장착 상태에서 무장 상태

	EWS_Max			UMETA(DisplayName = "DefaultMAX")
};

/**
 * 
 */
UCLASS()
class RPG_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
	class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Combat")
	class UBoxComponent* CombatCollision;
	
	//EquipSound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sound")
	class USoundCue* EquipedSound;

	//Particle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particle")
	bool bIdleParticle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual void PostInitializeComponents() override;

	/**********************************/
	// Weapon Overlap, Collision 관련 //
	/**********************************/
	//무기의 기본 Sphere Collision. (장착 범위등)
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UFUNCTION(BlueprintCallable)
	void ActivateCollision(); //Collision을 활성화.

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision(); //Collision을 비활성화.

	//Collision이 활성화 된 후 Enemy를 쳤을때
	UFUNCTION()
	void CombatCollisionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	//Collision이 활성화 된 후 Enemy를 치고 난 뒤
	UFUNCTION()
	void CombatCollisionOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	//이 무기를 갖고있는 Player를 가져온다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	class ACharacter* WeaponOwner;

	FORCEINLINE void SetWeaponOwner(class ACharacter* Character) { WeaponOwner = Character; }
	FORCEINLINE ACharacter* GetWeaponOwner() { return WeaponOwner; }

	/*******************************/
	//---      Weapon Stats    ---//
	/*******************************/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Stats")
	float WeaponDamage;

	/*******************************/
	//---      Weapon State     ---//
	/*******************************/
	//equip function
	void Equip(class ACharacter* Character);
	void Equip(class ACharacter* Character, const USkeletalMeshSocket* Socket);

	void ThrowWeapon(ACharacter* Character, FName SocketName);
	void ReceiveWeapon(ACharacter* Character);
	float Time;
	float AlphaTime;
	FTimerHandle WeaponThrowHandle;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
	EWeaponState WeaponState;

	FORCEINLINE void SetWeaponState(EWeaponState Status) { WeaponState = Status; }
	FORCEINLINE EWeaponState GetWeaponState() { return WeaponState; }
	
	

};
