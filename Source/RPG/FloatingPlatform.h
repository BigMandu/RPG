// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingPlatform.generated.h"

UCLASS()
class RPG_API AFloatingPlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloatingPlatform();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	class UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	FVector StartPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform", meta = (MakeEditWidget = "true"))
	FVector EndPoint;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	float InterpSpeed;*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	float InterpTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	bool bToggle;

	FTimerHandle PlatformTimer;

	//float MovingDistance;

	float Time;
	float AlphaTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	float RequireTime;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ToggleInterp();

	void SwapPoint(FVector& VectorX, FVector& VectorY);

};
