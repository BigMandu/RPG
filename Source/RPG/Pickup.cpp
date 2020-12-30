// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "MainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

APickup::APickup()
{
	CoinCount = 1;
	HealthPoint = 50.f;
}

void APickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	//UE_LOG(LogTemp, Warning, TEXT("Pickup::OnOverlap Begin"));
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar)
		{
			if (PickupType == EPickupType::EPT_Coin)
			{
				MainChar->IncrementCoin(CoinCount);
			}
			else if (PickupType == EPickupType::EPT_Potion)
			{
				MainChar->IncrementHealth(HealthPoint);
			}
			Destroy();
			if (OverlapParticle)
			{
				FVector PlayerLocation = MainChar->GetActorLocation();
				FVector PlayerOverLocation = FVector(PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z + 85.f);
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticle, PlayerOverLocation, FRotator(0.f), true);
			}
			if (OverlapSound)
			{
				UGameplayStatics::PlaySound2D(this, OverlapSound);
			}
		}
	}
}

void APickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	//UE_LOG(LogTemp, Warning, TEXT("Pickup::OnOverlap End"));
}