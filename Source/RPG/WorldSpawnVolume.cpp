// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldSpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy.h"
#include "EnemyAIController.h"


// Sets default values
AWorldSpawnVolume::AWorldSpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("WorldSpawnVolume"));
	RootComponent = SpawnVolume;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(GetRootComponent());


	SpawnActor_1 = nullptr;
	SpawnActor_2 = nullptr;
	SpawnActor_3 = nullptr;
	SpawnActor_4 = nullptr;

	SpawnCount = 1;
}

// Called when the game starts or when spawned
void AWorldSpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
	//if (SpawnActor_1 || SpawnActor_2 || SpawnActor_3 || SpawnActor_4)
	{
		SpawnArray.Add(SpawnActor_1);
		SpawnArray.Add(SpawnActor_2);
		SpawnArray.Add(SpawnActor_3);
		SpawnArray.Add(SpawnActor_4);
	}

	SpawnSelectedActor();
}

// Called every frame
//void AWorldSpawnVolume::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

FVector AWorldSpawnVolume::GetSpawnPoint()
{
	FVector VolumeSize = SpawnVolume->GetScaledBoxExtent();
	FVector VolumeLocation = SpawnVolume->GetComponentLocation();

	//박스 위치/사이즈내에 랜덤한 좌표값을 얻음
	FVector SpawnPoint = UKismetMathLibrary::RandomPointInBoundingBox(VolumeLocation, VolumeSize);

	return SpawnPoint;
}

TSubclassOf<AActor> AWorldSpawnVolume::GetSpawnActor()
{
	if (SpawnArray.Num() > 0)
	{
		for (int32 i = 0; i < SpawnCount; i++)
		{
			int32 Select = FMath::RandRange(0, SpawnArray.Num() - 1);
			if (SpawnArray[Select] != nullptr)
			{
				return SpawnArray[Select];
			}
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("Spawn Array is null"));
	return nullptr;
}

void AWorldSpawnVolume::SpawnSelectedActor()
{
	UWorld* World = GetWorld();

	if (World)
	{
		for (int32 i = 0; i < SpawnCount; i++)
		{
			AActor* Actor = World->SpawnActor<AActor>(GetSpawnActor(), GetSpawnPoint(), FRotator(0.f));

			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy) //Enemy일 경우 AIController를 넣어줌.
			{
				Enemy->SpawnDefaultController(); //AIController for this and actually set it for pawn.
				AEnemyAIController* AICon = Cast<AEnemyAIController>(Enemy->GetController());
				if (AICon)
				{
					Enemy->AIControllerClass = AICon->StaticClass();
					//Enemy->AIController = AICon;
				}
			}
		}
	}
}
