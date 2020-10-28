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

	//Begin play�� HUD�� �������.
	virtual void BeginPlay() override; //�ٸ� Ŭ����ó�� �����ش�.
};
