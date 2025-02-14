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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* PauseMenu;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<UUserWidget> WStorePage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* StorePage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<UUserWidget> WGameEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* GameEnd;



	virtual void Tick(float DeltaTime) override;
protected:
	//Begin play시 HUD를 띄워야함.
	virtual void BeginPlay() override; //다른 클래스처럼 맞춰준다.

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


	bool bGameEndWidgetVisible;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Store")
	void DisplayGameEndWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Store")
	void RemoveGameEndWidget();


};
