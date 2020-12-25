// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

//���� ����
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Spawn		UMETA(DisplayName = "Spawn"), //world�� pickup���·� spawn�� ����
	EWS_Equipped	UMETA(DisplayName = "Equipped"), //E key�� ������ ������ ����
	EWS_Sheath		UMETA(DisplayName = "Sheath"), //���� ���¿��� ���� ����
	EWS_Draw		UMETA(DisplayName = "Draw"), //���� ���¿��� ���� ����

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
	// Weapon Overlap, Collision ���� //
	/**********************************/
	//������ �⺻ Sphere Collision. (���� ������)
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UFUNCTION(BlueprintCallable)
	void ActivateCollision(); //Collision�� Ȱ��ȭ.

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision(); //Collision�� ��Ȱ��ȭ.

	//Collision�� Ȱ��ȭ �� �� Enemy�� ������
	UFUNCTION()
	void CombatCollisionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	//Collision�� Ȱ��ȭ �� �� Enemy�� ġ�� �� ��
	UFUNCTION()
	void CombatCollisionOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	//�� ���⸦ �����ִ� Player�� �����´�.
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
