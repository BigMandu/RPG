// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "MainCharacter.h"
#include "Enemy.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
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


void AWeapon::Equip(class AMainCharacter* MainChar)
{
	if (MainChar)
	{
		//Camera�� �����ϵ��� ����
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetSimulatePhysics(false); //Main Character�� Attach�����ֱ� ���� physics�� ��.
		const USkeletalMeshSocket* RightWeaponSocket = MainChar->GetMesh()->GetSocketByName("hand_r_weapon");

		if (RightWeaponSocket)
		{
			bRotate = false;
			//SetWeaponState(EWeaponState::EWS_Equipped);
			RightWeaponSocket->AttachActor(this, MainChar->GetMesh());
			MainChar->SetEquippedWeapon(this); //Main�� SetEquipped Weapon ȣ��.
			MainChar->SetActiveOverlappingItem(nullptr);
		}
		if (EquipedSound)
		{
			UGameplayStatics::PlaySound2D(this, EquipedSound);
		}
		if(bIdleParticle)
		{ 
			IdleParticle->Deactivate();
		}
	}
}

void AWeapon::CombatCollisionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (SweepResult.Actor.IsValid())
		{
			AEnemy* Enemy = Cast<AEnemy>(OtherActor);
			AMainCharacter* MainChar = GetWeaponOwner();
			if (Enemy && MainChar)
			{
				UE_LOG(LogTemp, Warning, TEXT("Weapon::Overlap Actor is Enemy"));
				if (Enemy->HitParticle)
				{
					FVector HitLocation = SweepResult.ImpactPoint;
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticle, HitLocation);
				}
				if (Enemy->HitSound)
				{
					UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
				}
				MainChar->AttackDamage(Enemy, WeaponDamage);
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