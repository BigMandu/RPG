// Fill out your copyright notice in the Description page of Project Settings.


#include "Store.h"
#include "MainCharacter.h"
#include "MainPlayerController.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

// Sets default values
AStore::AStore()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	StoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StoreMesh;

	StoreOverlap = CreateDefaultSubobject<USphereComponent>(TEXT("StoreOverlap"));
	StoreOverlap->SetupAttachment(GetRootComponent());
	//StoreOverlap->SetupAttachment(GetRootComponent());

	OverlapParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("OverlapParticle"));
	OverlapParticle->SetupAttachment(GetRootComponent());

	StoreOverlap->OnComponentBeginOverlap.AddDynamic(this, &AStore::OnOverlapBegin);
	StoreOverlap->OnComponentEndOverlap.AddDynamic(this, &AStore::OnOverlapEnd);
}

// Called when the game starts or when spawned
void AStore::BeginPlay()
{
	Super::BeginPlay();
	
	bIsOverlap = false;

	if (WStorePage)
	{
		StorePage = CreateWidget<UUserWidget>(GetWorld(), WStorePage);
		if (StorePage)
		{
			StorePage->AddToViewport();
			StorePage->SetVisibility(ESlateVisibility::Hidden);
			bIsStorePageVisible = false;
		}
	}
	if (OverlapParticle && OverlapSoundCue)
	{
		OverlapSound = UGameplayStatics::SpawnSound2D(this, OverlapSoundCue);
		if (OverlapParticle->IsActive())
		{
			OverlapParticle->Deactivate();
		}
		
		if (OverlapSound && OverlapSound->IsPlaying())
		{
			OverlapSound->Stop();
		}
		
	}
	
}

// Called every frame
void AStore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AStore::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT(" Store :: Overlap begin"));
	if (OtherActor)
	{
		Main = Cast<AMainCharacter>(OtherActor);
		if (Main == nullptr) return;

		Main->SetActiveOverlappingActor(this);
		bIsOverlap = true;

		if (OverlapParticle && OverlapSound)
		{
			if (OverlapParticle->IsActive() == false)
			{
				UE_LOG(LogTemp, Warning, TEXT("OverlapParticle is active"));
				OverlapParticle->ToggleActive();
			}
			if (OverlapSound->IsPlaying() == false)
			{
				UE_LOG(LogTemp, Warning, TEXT("OverlapSound is active"));
				OverlapSound->Play();
			}
			
		}
	}
}

void AStore::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT(" Store :: Overlap end"));
	if (OtherActor)
	{
		Main = Cast<AMainCharacter>(OtherActor);
		if (Main == nullptr) return;

		Main->SetActiveOverlappingActor(nullptr);
		bIsOverlap = false;
		if (OverlapParticle && OverlapSound)
		{
			if (OverlapParticle->IsActive())
			{
				UE_LOG(LogTemp, Warning, TEXT("OverlapParticle is Deactive"));
				OverlapParticle->ToggleActive();
			}
			if (OverlapSound->IsPlaying())
			{
				UE_LOG(LogTemp, Warning, TEXT("OverlapSound is Deactive"));
				OverlapSound->Stop();
			}
		}
	}
}

void AStore::DisplayStorePage_Implementation(ACharacter* OverlapCharacter)
{
	if (OverlapCharacter == nullptr) return;
	
	if (Main == nullptr || PlayerCon == nullptr)
	{
		if (Main == nullptr)
		{
			Main = Cast<AMainCharacter>(OverlapCharacter);
		}
		else
		{
			if (Main == nullptr) return;
			PlayerCon = Cast<AMainPlayerController>(Main->GetController());
			if (PlayerCon == nullptr) return;
		}
	}

	Main = Cast<AMainCharacter>(OverlapCharacter);
	if (Main == nullptr) return;
	
	PlayerCon = Cast<AMainPlayerController>(Main->GetController());
	if (PlayerCon == nullptr) return;


	Main->SaveGame(false);
	UE_LOG(LogTemp, Warning, TEXT(" Store :: Popup store page && Savegame"));

	if (bIsOverlap == true && OverlapSound->IsPlaying())
	{
		OverlapSound->Stop();
	}
	
	if(StorePage)
	{ 
		bIsStorePageVisible = true;
		
		UGameplayStatics::SetGamePaused(this, true);
		StorePage->SetVisibility(ESlateVisibility::Visible);
		FInputModeGameAndUI Inputmode;
		PlayerCon->SetInputMode(Inputmode);
		PlayerCon->bShowMouseCursor = true;
	}
		
}
void AStore::RemoveStorePage_Implementation(ACharacter* OverlapCharacter)
{
	if (Main == nullptr || PlayerCon == nullptr)
	{
		if (Main == nullptr)
		{
			Main = Cast<AMainCharacter>(OverlapCharacter);
		}
		else
		{
			if (Main == nullptr) return;
			PlayerCon = Cast<AMainPlayerController>(Main->GetController());
			if (PlayerCon == nullptr) return;
		}
	}
	
	
	if (StorePage)
	{
		bIsStorePageVisible = false;
		FInputModeGameOnly Inputmode;
		PlayerCon->SetInputMode(Inputmode);
		PlayerCon->bShowMouseCursor = false;
		StorePage->SetVisibility(ESlateVisibility::Hidden);
		UGameplayStatics::SetGamePaused(this, false);

		Main->SaveGame(false); //상점 이용후 저장 및 로드를 해준다. (캐릭터 스텟이 있기때문)
		Main->LoadGame(false);
	}

	if (bIsOverlap == true && OverlapSound->IsPlaying() == false)
	{
		OverlapSound->Play();
	}

}
