// Fill out your copyright notice in the Description page of Project Settings.


#include "Store.h"
#include "MainCharacter.h"
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

	OverlapParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("OverlapParticle"));
	OverlapParticle->SetupAttachment(GetRootComponent());

	StoreOverlap->OnComponentBeginOverlap.AddDynamic(this, &AStore::OnOverlapBegin);
	StoreOverlap->OnComponentEndOverlap.AddDynamic(this, &AStore::OnOverlapEnd);
}

// Called when the game starts or when spawned
void AStore::BeginPlay()
{
	Super::BeginPlay();

	if (OverlapParticle)
	{
		if (OverlapParticle->IsActive())
		{
			OverlapParticle->Deactivate();
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
		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main)
		{
			if (OverlapSoundCue)
			{
				OverlapSound = UGameplayStatics::SpawnSound2D(this, OverlapSoundCue);
				OverlapSound->Play();
			}
			Main->SetActiveOverlappingActor(this);

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
}

void AStore::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT(" Store :: Overlap end"));
	if (OtherActor)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main)
		{
			if (OverlapSound == nullptr)
			{
				if (OverlapSoundCue)
				{
					OverlapSound = UGameplayStatics::SpawnSound2D(this, OverlapSoundCue);
					OverlapSound->Stop();
					//OverlapSound->bAutoDestroy = true;
				}
			}
			else
			{
				if (OverlapSound->IsPlaying() == false)
				{
					OverlapSound->Stop();
				}
				//OverlapSound->bAutoDestroy = true;
			}

			Main->SetActiveOverlappingActor(nullptr);

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
}