// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Store.generated.h"

UCLASS()
class RPG_API AStore : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStore();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Store)
	class USphereComponent* StoreOverlap;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Store)
	class UStaticMeshComponent* StoreMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Store)
	class UParticleSystemComponent* OverlapParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Store)
	class USoundCue* OverlapSoundCue;

	class UAudioComponent* OverlapSound;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
