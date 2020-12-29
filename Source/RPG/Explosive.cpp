// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "MainCharacter.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AExplosive::AExplosive()
{
	Damage = 25.f;
}

void AExplosive::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	UE_LOG(LogTemp, Warning, TEXT("Explosive::OnOverlap Begin"));

	if (OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("overlap actor is %s"), *(OtherActor->GetFName().ToString()));
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (MainChar || Enemy)
		{	
			UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, DamageType);
		}
		if (OverlapParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticle, GetActorLocation(), FRotator(0.f), true);
		}
		if (OverlapSound)
		{
			UGameplayStatics::PlaySound2D(this, OverlapSound);
		}
		Destroy();
	}
	

}

void AExplosive::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	UE_LOG(LogTemp, Warning, TEXT("Explosive::OnOverlap End"));
}

void AExplosive::Delete()
{
	if (OverlapParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticle, GetActorLocation(), FRotator(0.f), true);
	}
	if (OverlapSound)
	{
		UGameplayStatics::PlaySound2D(this, OverlapSound);
	}
	Destroy();
}