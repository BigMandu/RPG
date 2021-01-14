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
		
		UGameplayStatics::SetGamePaused(this, true); //얘를 먼저하니까 아래꺼가 싹다 무시됨.

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
		UGameplayStatics::SetGamePaused(this, false); //얘를 제일 마지막에 해야 메뉴가 다 사라지고 재개됨.
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
			Main->SaveGame(false); //상점 이용후 저장 및 로드를 해준다. (캐릭터 스텟이 있기때문)
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
