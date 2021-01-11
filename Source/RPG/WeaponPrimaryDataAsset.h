// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateBrush.h"
#include "WeaponPrimaryDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class RPG_API UWeaponPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Stats")
	TSubclassOf<class AWeapon> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Stats")
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Stats")
	FSlateBrush WeaponIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Stats")
	int32 CoinPrice;
};
