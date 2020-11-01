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

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	//EquipSound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sound")
	class USoundCue* EquipedSound;

	//Particle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particle")
	bool bIdleParticle;

	//equip function
	void Equip(class AMainCharacter* MainChar);

	/*******************************/
	//---      Weapon State     ---//
	/*******************************/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
	EWeaponState WeaponState;

	FORCEINLINE void SetWeaponState(EWeaponState Status) { WeaponState = Status; }
	FORCEINLINE EWeaponState GetWeaponState() { return WeaponState; }
};
