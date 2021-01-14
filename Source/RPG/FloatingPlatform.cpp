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
	EndPoint += StartPoint; //EndPoint�� Reletive coordinate�� WorldLocation�� StartPoint�� ���� EndPoint�� world��ǥ���� ������.

	bToggle = false;

	//Lerp�� ����ʿ� ���� �Ÿ��� ���� �ʿ䰡 ������.
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


		// ������ ���� VInterpTo
		//FVector CurLocation = GetActorLocation();
		//FVector InterpLocation = FMath::VInterpTo(CurLocation, EndPoint, DeltaTime, InterpSpeed);
		//SetActorLocation(InterpLocation);
		//float CurrentMovingDistance = (GetActorLocation() - StartPoint).Size(); //������ġ���� ������ġ�� ���� ������ �Ÿ��� ����.


		//VInterpTo�� ÷�� �����̵������� �� �������� Lerp�� ���������.
		FVector LerpLocation = FMath::Lerp(StartPoint, EndPoint, AlphaTime);
		SetActorLocation(LerpLocation);
		

		

		if(AlphaTime >= 1.0f) //if (MovingDistance - CurrentMovingDistance <= 1.f) //�̵��� �ѰŸ����� ������ �Ÿ��� �� ���� 1.f���ϸ� (���� �ٿ�)
		{
			ToggleInterp(); //����ؼ� �������� �����
			GetWorldTimerManager().SetTimer(PlatformTimer, this, &AFloatingPlatform::ToggleInterp, InterpTime); //�ٽ� InterpTime���� ����� �����̰� �ϰ�.
			SwapPoint(StartPoint, EndPoint); //Vector�� swap, reverse�� �����̰� �Ѵ�.
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
