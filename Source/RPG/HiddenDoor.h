// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HiddenDoor.generated.h"

UCLASS()
class RPG_API AHiddenDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHiddenDoor();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HiddenDoor")
	class UBoxComponent* TriggerCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenDoor")
	class UStaticMeshComponent* Switch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenDoor")
	UStaticMeshComponent* HiddenDoor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenDoor")
	class UParticleSystemComponent* OverlapParticle; //TriggerCollision에 overlap됐을때 해당 파티클 출력.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenDoor")
	class USoundCue* OverlapSoundCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenDoor")
	USoundCue* DoorSoundCue;




	FTimerHandle HiddenDoorHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HiddenDoor")
	FVector InitDoorLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HiddenDoor")
	FVector InitSwitchLocation;

	//Overlap이 끝났을때 door가 Hold할 시간. (이 시간 후로 door가 lower함.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenDoor")
	float SwitchTime;

	bool bIsPlayerOverlap;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "HiddenDoor")
	void UpdateDoorLocation(float Z);

	UFUNCTION(BlueprintCallable, Category = "HiddenDoor")
	void UpdateFloorSwitchLocation(float Z);

	void CloseDoor();

	UFUNCTION(BlueprintImplementableEvent, Category = "HiddenDoor")
	void RaiseDoor();
	UFUNCTION(BlueprintImplementableEvent, Category = "HiddenDoor")
	void LowerDoor();

	UFUNCTION(BlueprintImplementableEvent, Category = "HiddenDoor")
	void RaiseFloorSwitch();
	UFUNCTION(BlueprintImplementableEvent, Category = "HiddenDoor")
	void LowerFloorSwitch();

};
