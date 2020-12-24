// Fill out your copyright notice in the Description page of Project Settings.


#include "Soul.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "MainCharacter.h"

// Sets default values
ASoul::ASoul()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());


	//Editor���� ���� ������.
	/*SoulParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SoulParticle"));
	SoulParticle->SetupAttachment(GetRootComponent());*/
	
	Time = 0.f;
	AlphaTime = 0.f;

	bInterp = false;
	SoulCount = 1;	
	
}

// Called when the game starts or when spawned
void ASoul::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle InterpTimer;
	InitialLocation = GetActorLocation();

	Sphere->SetSimulatePhysics(false); //���������� ���Ѵ�.

	if (IdleSound)
	{
		UGameplayStatics::PlaySound2D(this, IdleSound);
	}

	MainPlayer = Cast<AMainCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());

	if (MainPlayer == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainPlayer is nullptr"));
	}

	GetWorldTimerManager().SetTimer(InterpTimer, this, &ASoul::ToggleInterp, 1.5f);
	
}

// Called every frame
void ASoul::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bInterp && !MainPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Soul::Tick Error"));
		return;
	}
	
	FVector PlayerCurLocation = MainPlayer->GetActorLocation();
	
	//DeltaTime ��� ������. (����ð��� ��� ����)
	Time += DeltaTime;

	//Lerp�� �� Alpha�� ����ð� / �� �ð�(Player�� ���� �ð�)
	AlphaTime= Time / 1.5f;
	
	FVector LerpLocation = FMath::Lerp(InitialLocation, PlayerCurLocation, AlphaTime);
	
	//������
	/*UE_LOG(LogTemp, Warning, TEXT("AlphaTime : %f"), AlphaTime);
	UE_LOG(LogTemp, Warning, TEXT("PlayerLocation : %s"), *PlayerCurLocation.ToString());
	UE_LOG(LogTemp, Warning, TEXT("Lerp Location : %s"), *LerpLocation.ToString());*/
	
	//FVector InterpLocation = FMath::VInterpTo(InitialLocation, LerpLocation, DeltaTime, 2.0f);
	SetActorLocation(LerpLocation);

	//AlphaTime�� 1�̵Ǹ�(Player�� ������)
	if (AlphaTime>= 1.0f)
	{
		CollectSoul();
	}

}
//IncrementSoul 
void ASoul::CollectSoul()
{
	if (DestorySound)
	{
		UGameplayStatics::PlaySound2D(this, DestorySound);
	}
	MainPlayer->IncrementSoul(SoulCount);
	
	Destroy();
}

void ASoul::ToggleInterp()
{
	bInterp = true;
}