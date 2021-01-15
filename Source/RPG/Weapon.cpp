// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "MainCharacter.h"
#include "Enemy.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "WeaponPrimaryDataAsset.h"



AWeapon::AWeapon()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetRootComponent());

	WeaponState = EWeaponState::EWS_Spawn;
	WeaponDamage = 15.f;

	
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AWeapon::PostInitializeComponents() //AddDynamic�� ���⼭ ������.
{
	Super::PostInitializeComponents();
	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::CombatCollisionOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::CombatCollisionOverlapEnd);
	
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�⺻������ �浹�� ����.

	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic); //�浹 ä���� worlddynamicä�η� �����Ѵ�. (�����̴� ���Ͷ�)

	CombatCollision->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); //Pawn�� ���� �浹�� overlap.
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block); //Staticmesh�� ������ϰ� �ѹ� �غô�.

}


void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (OtherActor && (WeaponState == EWeaponState::EWS_Spawn)) //Weapon�� Spawn���°� �ƴϸ� ������ �ʿ䰡 ����.
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar)
		{
			MainChar->SetActiveOverlappingActor(this);
			
			if (MainChar->EquippedWeapon != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("EquippedWeapon is exist"));
				UE_LOG(LogTemp, Warning, TEXT("Equipped Weapon Damage is : %f,  This Weapon Damage is : %f"), MainChar->EquippedWeapon->WeaponDamage, WeaponDamage);

			/* OnOverlap�� ������, ������ equipped weapon�� �ִٸ�,
			���� Weapon�� Damage����, ���� Overlap�� Weapon�� Damage���� ����ְ�
			������ Damage�� ���� ������ �� �ְ� ����.*/
			}
			
		}
	}
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (OtherActor)
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar)
		{
			MainChar->SetActiveOverlappingActor(nullptr);
		}
	}
}


void AWeapon::Equip(class ACharacter* Character)
{
	AMainCharacter* MainChar = Cast<AMainCharacter>(Character);

	if (MainChar)
	{
		//Camera�� �����ϵ��� ����
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetSimulatePhysics(false); //Main Character�� Attach�����ֱ� ���� physics�� ��.
		CollisionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		const USkeletalMeshSocket* RightWeaponSocket = MainChar->GetMesh()->GetSocketByName("hand_r_weapon");

		if (EquipedSound)
		{
			if (WeaponState == EWeaponState::EWS_Equipped)  //Į�� ������ �ٽ� ������ ����Ǵ� �Ҹ� ���� �ص�.
			{
				UGameplayStatics::PlaySound2D(this, AbilityReceiveSound);
			}
			else
			{
				UGameplayStatics::PlaySound2D(this, EquipedSound);
			}
		}

		if(bIdleParticle)
		{ 
			IdleParticle->Deactivate();
		}
		if (RightWeaponSocket)
		{
			SetWeaponState(EWeaponState::EWS_Equipped);
			bRotate = false;
			//SetWeaponState(EWeaponState::EWS_Equipped);
			RightWeaponSocket->AttachActor(this, MainChar->GetMesh());
			MainChar->SetEquippedWeapon(this); //Main�� SetEquipped Weapon ȣ��.
			MainChar->SetActiveOverlappingActor(nullptr);
		}

		
		if (GetWorldTimerManager().IsTimerActive(WeaponReceiveHandle))
		{
			GetWorldTimerManager().ClearTimer(WeaponReceiveHandle);
		}
	}

	
}

void AWeapon::Equip(class ACharacter* Character, const USkeletalMeshSocket* Socket)
{
	AEnemy* Enemy = Cast<AEnemy>(Character);
	if (Enemy && Socket)
	{
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		CollisionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Socket->AttachActor(this, Enemy->GetMesh());
	}
}

void AWeapon::ThrowWeapon(ACharacter* Character, FName SocketName, float AbilityDistance, float AbilityRotation)
{
	AMainCharacter* Main = Cast<AMainCharacter>(Character);
	if (Main)
	{
		this->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
		
		FVector BeforeBoxExtent = CombatCollision->GetUnscaledBoxExtent(); //�������� �ڽ�ũ�⸦ ����.
		FVector ScaledBox = FVector(BeforeBoxExtent.X * 4, BeforeBoxExtent.Y * 2, BeforeBoxExtent.Z * 2.5); //�ڽ��� ũ�⸦ ũ�� ���ش�.

		CombatCollision->SetBoxExtent(ScaledBox); //�ڽ�ũ�⸦ �÷���
		CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		//�ش� �����Ƽ ���� ��ź������ ������ �� �ֵ��� �Ѵ�.
		CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

		FVector Destination = (Main->GetActorForwardVector() * AbilityDistance) + Main->GetActorLocation();
		Destination.Z = GetActorLocation().Z;
		FVector CurWeaponLocation = GetActorLocation();

		//FRotator InitRotation = GetActorRotation();

		Time = 0.f;
		AlphaTime = 0.f;		

		GetWorldTimerManager().SetTimer(WeaponThrowHandle, [=] {
			Time += GetWorld()->GetDeltaSeconds();
			AlphaTime = Time / 1.f;
			
			//������
			//if (GEngine)
			//{
			//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Time : %f , AlphaTime : %f"), Time, AlphaTime ));
			//	UE_LOG(LogTemp, Warning, TEXT("Time : %f , AlphaTime : %f"), Time, AlphaTime);
			//}

			FVector WeaponLocation = FMath::Lerp(CurWeaponLocation, Destination, AlphaTime); //AlphaTime���� ��ġ�� ��Ÿ����.
			SetActorLocation(WeaponLocation);  //���� ��ġ�� ����ؼ� ������Ʈ ���ش�.

			FRotator WeaponRolling = FRotator(90.f, 0.f, Time * AbilityRotation);  //vertical�� �����ϰ� �ϰ� ȸ���ϰ� �Ѵ�.
			//UE_LOG(LogTemp, Warning, TEXT("AbilityRotation : %f"), AbilityRotation);
			SetActorRotation(WeaponRolling);

			if (AbilityThrowSound && GetActorRotation().Roll >= 0.f)
			{	
				UGameplayStatics::PlaySound2D(this, AbilityThrowSound);	
			}

			if (AlphaTime >= 1.f) //AlphaTime�� 1�϶� ����.
			{
				//UE_LOG(LogTemp, Warning, TEXT("Alpha Time is over 1.f"));
				ReceiveWeapon(Main, BeforeBoxExtent); //���� �ڽ�ũ�⵵ �Ѱ��ش�.
			}
			
			}, GetWorld()->GetDeltaSeconds(), true);
		
		

	}
}

void AWeapon::ReceiveWeapon(ACharacter* Character, FVector BoxExtent)
{
	AMainCharacter* Main = Cast<AMainCharacter>(Character);
	if (Main)
	{
		GetWorldTimerManager().ClearTimer(WeaponThrowHandle);
		
		Time = 0.f;
		AlphaTime = 0.f;

		FVector WeaponLocation = GetActorLocation();


		CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap); //���ƿö� ���� ��ź������ ������ �� �ֵ��� �Ѵ�.

		GetWorldTimerManager().SetTimer(WeaponReceiveHandle, [=] {
			Time += GetWorld()->GetDeltaSeconds();
			AlphaTime = Time / 1.0f;
			/*if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Time : %f , AlphaTime : %f"), Time, AlphaTime));
				UE_LOG(LogTemp, Warning, TEXT("Time : %f , AlphaTime : %f"), Time, AlphaTime);
			}*/
			FVector MainLocation = Main->GetActorLocation();
			FVector MainOverLocation = FVector(MainLocation.X, MainLocation.Y + 40.f, MainLocation.Z + 80.f); //�÷��̾��� ����/�Ӹ����� �����ϰ� �Ѵ�. �ڿ������� ������� �̾�����.

			FVector TargetLocation = FMath::Lerp(WeaponLocation, MainOverLocation, AlphaTime);
			SetActorLocation(TargetLocation);

			FRotator WeaponRolling = FRotator(90.f, 0.f, Time*2048.f);
			SetActorRotation(WeaponRolling);

			if (AbilityThrowSound && GetActorRotation().Roll >= 0.f)
			{
				UGameplayStatics::PlaySound2D(this, AbilityThrowSound);
			}
			if (AlphaTime >= 0.9f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Alpha Time is over 0.9"));
				SetActorRotation(FRotator(75.f, 0.f, 0.f));

				
				CombatCollision->SetBoxExtent(BoxExtent); //���� ũ��� ������.
				CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //�ٽ� ��Ȱ��ȭ�� ���ش�.

				//Collision ���� �ʱ�ȭ
				CombatCollision->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
				CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
				CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

				Equip(Main);
				Main->Ability_ThrowWeapon_Finish();
			}

			}, GetWorld()->GetDeltaSeconds(), true);		
	}
}

void AWeapon::CombatCollisionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (SweepResult.Actor.IsValid()) 
		{
			AMainCharacter* OwnerMain = Cast<AMainCharacter>(GetWeaponOwner()); //�����ڰ� Player
			AEnemy* OwnerEnemy = Cast<AEnemy>(GetWeaponOwner()); //�����ڰ� Enemy

			if (OwnerMain) //�����ڰ� Player�϶�(�� ������ �����ڰ� Player�϶�)
			{
				AEnemy* Enemy = Cast<AEnemy>(OtherActor); //������
				if (Enemy)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Weapon::Overlap Actor is Enemy"));
					if (Enemy->HitParticle)
					{
						FVector EnemyLocation = Enemy->GetActorLocation();
						
						FVector HitLocation = FVector(EnemyLocation.X, EnemyLocation.Y, EnemyLocation.Z + Enemy->GetDefaultHalfHeight());

						UE_LOG(LogTemp, Warning, TEXT("Enemy World location is : %s"), *Enemy->GetActorLocation().ToString());
						UE_LOG(LogTemp, Warning, TEXT("HitLocation is : %s"), *HitLocation.ToString());
						/*
						UE_LOG(LogTemp, Warning, TEXT("Enemy World location is : %s"), *Enemy->GetActorLocation().ToString());
						UE_LOG(LogTemp, Warning, TEXT("Hit Actor Name is : %s"), *SweepResult.GetActor()->GetFName().ToString());
						UE_LOG(LogTemp, Warning, TEXT("Hit Bone Name is : %s"), *SweepResult.BoneName.ToString());
						UE_LOG(LogTemp, Warning, TEXT("Impact Point is : %s"), *HitLocation.ToString());
						UE_LOG(LogTemp, Warning, TEXT("Impact Normal is : %s"), *SweepResult.ImpactNormal.ToString());
						UE_LOG(LogTemp, Warning, TEXT("Location is : %s"), *SweepResult.Location.ToString());
						UE_LOG(LogTemp, Warning, TEXT("Normal is : %s"), *SweepResult.Normal.ToString());
						*/
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticle, HitLocation);
					}
					if (Enemy->HitSound)
					{
						UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
					}
					OwnerMain->AttackGiveDamage(Enemy, WeaponDamage);
				}
			}

			if (OwnerEnemy)
			{
				AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
				if (MainChar)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Weapon::Enemy Hit Player!"));
					OwnerEnemy->AttackGiveDamage(MainChar);
				}
			}
		}
	}
}

void AWeapon::CombatCollisionOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

void AWeapon::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeapon::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}