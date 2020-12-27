// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "MainCharacter.h"
#include "Enemy.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"



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
			MainChar->SetActiveOverlappingItem(this);
			
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
			MainChar->SetActiveOverlappingItem(nullptr);
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
			MainChar->SetActiveOverlappingItem(nullptr);
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

void AWeapon::ThrowWeapon(ACharacter* Character, FName SocketName)
{
	AMainCharacter* Main = Cast<AMainCharacter>(Character);
	if (Main)
	{
		this->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
		
		//CollisionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		FVector Destination = (Main->GetActorForwardVector() * 1600.f) + Main->GetActorLocation();
		Destination.Z = GetActorLocation().Z;
		FVector CurWeaponLocation = GetActorLocation();

		FRotator InitRotation = GetActorRotation();

		Time = 0.f;
		AlphaTime = 0.f;

		GetWorldTimerManager().SetTimer(WeaponThrowHandle, [=] {
			Time += GetWorld()->GetDeltaSeconds();
			AlphaTime = Time / 1.f;
			if (AbilityThrowSound)
			{
				UGameplayStatics::PlaySound2D(this, AbilityThrowSound);
			}

			FVector WeaponLocation = FMath::Lerp(CurWeaponLocation, Destination, AlphaTime); //AlphaTime���� ��ġ�� ��Ÿ����.
			SetActorLocation(WeaponLocation);  //���� ��ġ�� ����ؼ� ������Ʈ ���ش�.

			FRotator WeaponRotation = GetActorRotation();
			WeaponRotation.Roll += GetWorld()->GetDeltaSeconds() * 2000.f; //2000�ӵ��� �ӵ��� ȸ���ϰ� �Ѵ�.
			FRotator WeaponRolling = FRotator(90.f, 0.f, WeaponRotation.Roll);  //vertical�� �����ϰ� �ϰ� ȸ���ϰ� �Ѵ�.
			SetActorRotation(WeaponRolling);

			if (AlphaTime >= 1.f) //AlphaTime�� 1�϶� ����.
			{
				UE_LOG(LogTemp, Warning, TEXT("Alpha Time is over 1.f"));
				ReceiveWeapon(Main);
			}
			
			}, GetWorld()->GetDeltaSeconds(), true);
		
		

	}
}

void AWeapon::ReceiveWeapon(ACharacter* Character)
{
	AMainCharacter* Main = Cast<AMainCharacter>(Character);
	if (Main)
	{
		Main->Ability_ThrowWeapon_Finish();
		GetWorldTimerManager().ClearTimer(WeaponThrowHandle);
		//SetActorRotation(InitRotation);
		SetActorLocation(Main->GetActorLocation());
		Equip(Main);
		CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
					UE_LOG(LogTemp, Warning, TEXT("Weapon::Overlap Actor is Enemy"));
					if (Enemy->HitParticle)
					{
						FVector HitLocation = SweepResult.ImpactPoint; //���� �ȵ�. 
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
					UE_LOG(LogTemp, Warning, TEXT("Weapon::Enemy Hit Player!"));
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