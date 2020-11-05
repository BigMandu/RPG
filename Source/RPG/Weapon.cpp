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

	if (OtherActor && (WeaponState == EWeaponState::EWS_Spawn)) //Weapon이 Spawn상태가 아니면 수행할 필요가 없음.
	{
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if (MainChar)
		{
			MainChar->SetActiveOverlappingItem(this);
			
			/* OnOverlap이 됐을때, 기존에 equipped weapon이 있다면,
			기존 Weapon의 Damage값과, 지금 Overlap된 Weapon의 Damage값을 띄워주고
			유저가 Damage를 보고 선택할 수 있게 하자.*/
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
		//Camera를 무시하도록 설정
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetSimulatePhysics(false); //Main Character와 Attach시켜주기 위해 physics를 끔.

		const USkeletalMeshSocket* RightWeaponSocket = MainChar->GetMesh()->GetSocketByName("hand_r_weapon");
		if (RightWeaponSocket)
		{
			bRotate = false;
			RightWeaponSocket->AttachActor(this, MainChar->GetMesh());
			MainChar->SetEquippedWeapon(this); //Main의 SetEquipped Weapon 호출.
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