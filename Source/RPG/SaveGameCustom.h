// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameCustom.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FCharacterStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	float MaxHealth;
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	float Health;
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	float MaxStamina;
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	float Stamina;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	int32 Coins;
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	int32 Souls;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	float PlayerDamage;
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	float ThrowAbility_Distance;
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	float SmashAbility_Damage;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	FString SaveWeaponName;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	FString MapName;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	FVector Location;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData");
	FRotator Rotation;
};

UCLASS()
class RPG_API USaveGameCustom : public USaveGame
{
	GENERATED_BODY()
public:
	USaveGameCustom();

	UPROPERTY(VisibleAnywhere, Category = Basic) //Basic으로 지정해줘야 저장이 된다.
	FString SlotName;
	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FCharacterStats SaveCharacterStats;
};
