// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	//UMG에 대한 참조 (Unreal Mothing Graphics)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<class UUserWidget> HUDOverlayAsset; //PlayerController의 BP에서 이 항목을 선택할 수 있도록.

	//Widget을 생성하고 저장할 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* HUDOverlay; //위의 항목에서 이것을 선택할 수 있게 한다.



	//Enemy HealthBar

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	//TSubclassOf<UUserWidget> WEnemyHealthBar;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	//UUserWidget* EnemyHealthBar;
	//
	//FVector EnemyLocation;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets)
	//bool bEnemyHealthBarVisible;

	//void ShowEnemyHealthBar();
	//void HideEnemyHealthBar();
	/////////////////////////////

	virtual void Tick(float DeltaTime) override;
protected:
	//Begin play시 HUD를 띄워야함.
	virtual void BeginPlay() override; //다른 클래스처럼 맞춰준다.
	
};
