// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyWidget.generated.h"


/**
 * 
 */
UCLASS()
class RPG_API UEnemyWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	/*virtual void NativeConstruct() override;
	void UpdateHP();*/
public:

	float EnemyMaxHP;
	float EnemyHP;

	UFUNCTION(BlueprintCallable)
	float SetHPRatio();

	/*UPROPERTY()
	class UProgressBar* HPbarProgress;*/

};
