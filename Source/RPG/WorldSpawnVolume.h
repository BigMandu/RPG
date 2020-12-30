// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldSpawnVolume.generated.h"

UCLASS()
class RPG_API AWorldSpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldSpawnVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnVolume")
	class UBoxComponent* SpawnVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnVolume")
	class UBillboardComponent* Billboard;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnVolume")
	TSubclassOf<AActor> SpawnActor_1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnVolume")
	TSubclassOf<AActor> SpawnActor_2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnVolume")
	TSubclassOf<AActor> SpawnActor_3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnVolume")
	TSubclassOf<AActor> SpawnActor_4;

	TArray<TSubclassOf<AActor>>SpawnArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnVolume")
	int32 SpawnCount;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	FVector GetSpawnPoint();

	UFUNCTION()
	TSubclassOf<AActor> GetSpawnActor();	

	UFUNCTION()
	void SpawnSelectedActor();

};
