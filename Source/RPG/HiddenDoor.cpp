// Fill out your copyright notice in the Description page of Project Settings.


#include "HiddenDoor.h"
#include "MainCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AHiddenDoor::AHiddenDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerCollision;
	
	Switch = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Switch"));
	Switch->SetupAttachment(GetRootComponent());

	HiddenDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HiddenDoor"));
	HiddenDoor->SetupAttachment(GetRootComponent());

	OverlapParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("OverlapParticle"));
	OverlapParticle->SetupAttachment(GetRootComponent());

	SwitchTime = 3.f; 
	bIsPlayerOverlap = false;


	//트리거 충돌 세팅
	TriggerCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	TriggerCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

}

// Called when the game starts or when spawned
void AHiddenDoor::BeginPlay()
{
	Super::BeginPlay();

	TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &AHiddenDoor::OnOverlapBegin);
	TriggerCollision->OnComponentEndOverlap.AddDynamic(this, &AHiddenDoor::OnOverlapEnd);

	InitSwitchLocation = Switch->GetComponentLocation();
	InitDoorLocation = HiddenDoor->GetComponentLocation();

	if (OverlapParticle)
	{
		OverlapParticle->Activate();
	}
}

// Called every frame
void AHiddenDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHiddenDoor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar)
		{
			if (OverlapParticle && OverlapSoundCue)
			{
				OverlapParticle->Deactivate();
				UGameplayStatics::PlaySound2D(this, OverlapSoundCue);
			}
			if (DoorSoundCue)
			{
				UGameplayStatics::PlaySound2D(this, DoorSoundCue);
			}
			bIsPlayerOverlap = true;
			RaiseDoor();
			LowerFloorSwitch();

		}
	}
	
}

void AHiddenDoor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar)
		{
			if (OverlapParticle->IsActive()==false)
			{
				OverlapParticle->Activate();
			}
			bIsPlayerOverlap = false;
			GetWorldTimerManager().SetTimer(HiddenDoorHandle, this, &AHiddenDoor::CloseDoor, SwitchTime);
		}
	}

}

void AHiddenDoor::CloseDoor()
{
	if (bIsPlayerOverlap == false)
	{
		if (DoorSoundCue)
		{
			UGameplayStatics::PlaySound2D(this, DoorSoundCue);
		}
		LowerDoor();
		RaiseFloorSwitch();
	}
}

void AHiddenDoor::UpdateDoorLocation(float Z) //Blueprint Time라인으로 매프레임마다 Z값을 생성, 해당 함수의 파라미터로 넘겨준다.
{
	FVector NewLocation = InitDoorLocation;
	NewLocation.Z += Z;
	HiddenDoor->SetWorldLocation(NewLocation);
}


void AHiddenDoor::UpdateFloorSwitchLocation(float Z)
{
	FVector NewLocation = InitSwitchLocation;
	NewLocation.Z += Z;
	Switch->SetWorldLocation(NewLocation);
}
