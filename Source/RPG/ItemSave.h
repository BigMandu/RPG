// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSave.generated.h"

UCLASS()
class RPG_API AItemSave : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemSave();

	UPROPERTY(EditDefaultsOnly, Category = "SaveGameData")
	TMap<FString, TSubclassOf<class AWeapon>> WeaponData; //Blueprint·Î ³Ö¾îÁáÀ½ -> String Actor

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

};
