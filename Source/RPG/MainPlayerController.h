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

	//UMG�� ���� ���� (Unreal Mothing Graphics)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<class UUserWidget> HUDOverlayAsset; //PlayerController�� BP���� �� �׸��� ������ �� �ֵ���.

	//Widget�� �����ϰ� ������ ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* HUDOverlay; //���� �׸񿡼� �̰��� ������ �� �ְ� �Ѵ�.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* PauseMenu;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<UUserWidget> WStorePage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* StorePage;

	virtual void Tick(float DeltaTime) override;
protected:
	//Begin play�� HUD�� �������.
	virtual void BeginPlay() override; //�ٸ� Ŭ����ó�� �����ش�.

public:
	bool bPauseMenuVisible;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PauseMenu")
	void DisplayPauseMenu();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PauseMenu")
	void RemovePauseMenu();

	void TogglePauseMenu();
	

	bool bIsStorePageVisible;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Store")
	void DisplayStorePage();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Store")
	void RemoveStorePage();
};
