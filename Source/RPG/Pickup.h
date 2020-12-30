// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Pickup.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EPickupType : uint8
{
	EPT_Potion	UMETA(DisplayName = "Potion"),
	EPT_Coin	UMETA(DisplayName = "Coin"),

	EPT_MAX		UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class RPG_API APickup : public AItem
{
	GENERATED_BODY()
public:
	APickup();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pickup")
	EPickupType PickupType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup | Coin")
	int32 CoinCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup | Potion")
	float HealthPoint;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
};
