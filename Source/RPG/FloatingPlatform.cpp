// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingPlatform.h"


// Sets default values
AFloatingPlatform::AFloatingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;

	StartPoint = FVector(0.f);
	EndPoint = FVector(0.f);

	bToggle = false;
	InterpTime = 1.f;

	Time = 0.f;
	AlphaTime = 0.f;
	RequireTime = 2.0f;
}

// Called when the game starts or when spawned
void AFloatingPlatform::BeginPlay()
{
	Super::BeginPlay();
	
	//StaticMesh->SetSimulatePhysics(false);

	StartPoint = GetActorLocation();
	EndPoint += StartPoint; //EndPoint는 Reletive coordinate라 WorldLocation인 StartPoint에 더해 EndPoint의 world좌표값을 세팅함.

	bToggle = false;

	//Lerp로 변경됨에 따라 거리를 구할 필요가 없어짐.
	//MovingDistance = (EndPoint - StartPoint).Size();

	GetWorldTimerManager().SetTimer(PlatformTimer, this, &AFloatingPlatform::ToggleInterp, InterpTime);
}

// Called every frame
void AFloatingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bToggle)
	{
		Time += DeltaTime;
		AlphaTime = Time / RequireTime;

		UE_LOG(LogTemp, Warning, TEXT("DeltaTime : %.2f, Time : %.2f, AlphaTime : %.2f"), DeltaTime, Time, AlphaTime);


		// 기존에 사용된 VInterpTo
		//FVector CurLocation = GetActorLocation();
		//FVector InterpLocation = FMath::VInterpTo(CurLocation, EndPoint, DeltaTime, InterpSpeed);
		//SetActorLocation(InterpLocation);
		//float CurrentMovingDistance = (GetActorLocation() - StartPoint).Size(); //현재위치에서 시작위치를 빼서 움직인 거리를 구함.


		//VInterpTo가 첨에 순간이동식으로 팍 움직여서 Lerp로 변경해줬다.
		FVector LerpLocation = FMath::Lerp(StartPoint, EndPoint, AlphaTime);
		SetActorLocation(LerpLocation);
		

		

		if(AlphaTime >= 1.0f) //if (MovingDistance - CurrentMovingDistance <= 1.f) //이동할 총거리에서 움직인 거리를 뺀 값이 1.f이하면 (거의 다옴)
		{
			ToggleInterp(); //토글해서 움직임을 멈춘뒤
			GetWorldTimerManager().SetTimer(PlatformTimer, this, &AFloatingPlatform::ToggleInterp, InterpTime); //다시 InterpTime이후 토글해 움직이게 하고.
			SwapPoint(StartPoint, EndPoint); //Vector를 swap, reverse로 움직이게 한다.
		}
	}

}

void AFloatingPlatform::ToggleInterp()
{
	bToggle = !bToggle;
	Time = 0.f;
	AlphaTime = 0.f;
}

void AFloatingPlatform::SwapPoint(FVector& VectorX, FVector& VectorY)
{
	FVector TmpVector = VectorX;
	VectorX = VectorY;
	VectorY = TmpVector;
}
