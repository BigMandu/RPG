// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

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
	if (WPauseMenu)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
			bPauseMenuVisible = false;
		}
	}
}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		
		

		UGameplayStatics::SetGamePaused(this, true); //�긦 �����ϴϱ� �Ʒ����� �ϴ� ���õ�.

		PauseMenu->SetVisibility(ESlateVisibility::Visible);
		FInputModeGameAndUI InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
		UGameplayStatics::SetGamePaused(this, false); //�긦 ���� �������� �ؾ� �޴��� �� ������� �簳��.
	}
}

void AMainPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible)
	{
		RemovePauseMenu();
	}
	else
	{
		DisplayPauseMenu();
	}
}
