// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "MainCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"


AWeapon::AWeapon()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	WeaponState = EWeaponState::EWS_Spawn;
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
			
			/* OnOverlap�� ������, ������ equipped weapon�� �ִٸ�,
			���� Weapon�� Damage����, ���� Overlap�� Weapon�� Damage���� ����ְ�
			������ Damage�� ���� ������ �� �ְ� ����.*/
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