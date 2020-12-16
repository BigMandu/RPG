// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}

	//if (WEnemyHealthBar)
	//{
	//	EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
	//	if (EnemyHealthBar)
	//	{
	//		EnemyHealthBar->AddToViewport();
	//		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden); //처음에는 숨긴다. 특정 분기때 보여줌.
	//		
	//	}
	//}
	//bEnemyHealthBarVisible = false;
}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (EnemyHealthBar && bEnemyHealthBarVisible == true)
	{
		FVector2D HealthBarPosition;
		FVector2D HealthBarSize(300.f, 25.f);
		ProjectWorldLocationToScreen(EnemyLocation, HealthBarPosition);
		
		HealthBarPosition.X -= 85.f;
		HealthBarPosition.Y -= 90.f;		

		EnemyHealthBar->SetPositionInViewport(HealthBarPosition);
		EnemyHealthBar->SetDesiredSizeInViewport(HealthBarSize);
	}*/
}
//
//void AMainPlayerController::ShowEnemyHealthBar()
//{
//	if (EnemyHealthBar)
//	{
//		bEnemyHealthBarVisible = true;
//		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
//
//	}
//}
//
//void AMainPlayerController::HideEnemyHealthBar()
//{
//	if (EnemyHealthBar)
//	{
//		bEnemyHealthBarVisible = false;
//		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
//		
//	}
//}

