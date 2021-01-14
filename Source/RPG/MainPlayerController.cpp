// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "MainCharacter.h"
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

	if (WStorePage)
	{
		StorePage = CreateWidget<UUserWidget>(this, WStorePage);
		if (StorePage)
		{
			StorePage->AddToViewport();
			StorePage->SetVisibility(ESlateVisibility::Hidden);
			bIsStorePageVisible = false;
		}
	}

	if (WGameEnd)
	{
		GameEnd = CreateWidget<UUserWidget>(this, WGameEnd);
		if (GameEnd)
		{
			GameEnd->AddToViewport();
			GameEnd->SetVisibility(ESlateVisibility::Hidden);
			bGameEndWidgetVisible = false;
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



void AMainPlayerController::DisplayStorePage_Implementation()
{
	if (StorePage)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(GetPawn());
		if (Main)
		{
			Main->SaveGame(false);
			UE_LOG(LogTemp, Warning, TEXT("DisplayStorepage && Save game"));
		}
		
		bIsStorePageVisible = true;

		StorePage->SetVisibility(ESlateVisibility::Visible);

		//FInputModeGameAndUI Inputmode;
		FInputModeUIOnly Inputmode;
		SetInputMode(Inputmode);
		bShowMouseCursor = true;

		UGameplayStatics::SetGamePaused(this, true);
	}

}

void AMainPlayerController::RemoveStorePage_Implementation()
{
	if (StorePage)
	{
		
		AMainCharacter* Main = Cast<AMainCharacter>(GetPawn());
		if (Main)
		{
			Main->SaveGame(false); //���� �̿��� ���� �� �ε带 ���ش�. (ĳ���� ������ �ֱ⶧��)
			Main->LoadGame(false);
			UE_LOG(LogTemp, Warning, TEXT("RemoveStorepage && Save, Load game"));
		}
		
		
		bIsStorePageVisible = false;
		
		FInputModeGameOnly Inputmode;
		SetInputMode(Inputmode);
		

		bShowMouseCursor = false;

		StorePage->SetVisibility(ESlateVisibility::Hidden);

		UGameplayStatics::SetGamePaused(this, false);
	}
}

void AMainPlayerController::DisplayGameEndWidget_Implementation()
{
	bGameEndWidgetVisible = true;

	UGameplayStatics::SetGamePaused(this, true);
	
	GameEnd->SetVisibility(ESlateVisibility::Visible);
	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AMainPlayerController::RemoveGameEndWidget_Implementation()
{
	bGameEndWidgetVisible = false;

	GameEnd->SetVisibility(ESlateVisibility::Hidden);
}
