// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

UCLASS()
class RPG_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter();

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

	/*****************************/
	// Player Stats ����  //
	/*****************************/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats");
	float MaxHealth;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats");
	float Health;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats");
	float MaxStamina;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats");
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats");
	int32 Coins;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats");
	int32 Souls;
	/////////////////////////////

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//// Movement�Լ� ////
	void MoveForward(float Value);
	void MoveRight(float Value);

	//// ȸ�� ////
	//@param Rate ����ȭ ����. 1.0 == ���ϴ� ȸ���� 100%
	void TurnAtRate(float Rate);
	void LookUpRate(float Rate);

	////Damage���� �Լ�////
	void DecrementHealth(float Amount);
	void Die();

	////Coin, Soul function////
	void IncrementCoin(int32 Amount);
	//void DecrementCoin(int32 Amount);

	//void IncrementSoul(int32 Amount);
	//void DecrementSoul(int32 Amount);
};
