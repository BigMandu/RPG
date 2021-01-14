// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelTransitionVolume.h"
#include "MainCharacter.h"
#include "MainPlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"

// Sets default values
ALevelTransitionVolume::ALevelTransitionVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false; //Tick off

	TransitionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TransitionVolume"));
	RootComponent = TransitionVolume;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));

	Billboard->SetupAttachment(GetRootComponent());

	TransitionVolume->OnComponentBeginOverlap.AddDynamic(this, &ALevelTransitionVolume::OnOverlapBegin);

	bIsGameEndVolume = false;
}
 
// Called when the game starts or when spawned
void ALevelTransitionVolume::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALevelTransitionVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main)
		{
			if (bIsGameOverVolume) //해당 볼륨이 GameOver로 사용되면
			{
				Main->DecrementHealth(Main->MaxHealth); //플레이어의 체력만큼 닳게해서 죽게한뒤
				Main->PlayerController->DisplayPauseMenu(); //퍼즈 메뉴를 호출한다.
			}
			else if (bIsGameEndVolume) //해당 볼륨이 Game End 트리거로 사용됐을때는 Game end Widget을 보여준다.
			{
				Main->PlayerController->DisplayGameEndWidget();
			}
			else //그외엔 그냥 level transition volume
			{
				Main->SwitchLevel(TransitionLevelName);
			}
		}
	}
}