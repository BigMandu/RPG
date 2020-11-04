// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelTransitionVolume.generated.h"

UCLASS()
class RPG_API ALevelTransitionVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelTransitionVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Transition")
	class UBoxComponent* TransitionVolume;

	//Transition할 LevelName을 Editor에서 입력하게 한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Transition")
	FName TransitionLevelName;

	class UBillboardComponent* Billboard;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
