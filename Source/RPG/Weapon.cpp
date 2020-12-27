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

void AWeapon::PostInitializeComponents() //AddDynamic을 여기서 해주자.
{
	Super::PostInitializeComponents();
	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::CombatCollisionOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::CombatCollisionOverlapEnd);
	
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);//기본적으로 충돌을 끈다.

	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic); //충돌 채널을 worlddynamic채널로 설정한다. (움직이는 액터라)

	CombatCollision->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); //Pawn에 대한 충돌만 overlap.

	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block); //Staticmesh를 통과못하게 한번 해봤다.

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
			
			if (MainChar->EquippedWeapon != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("EquippedWeapon is exist"));
				UE_LOG(LogTemp, Warning, TEXT("Equipped Weapon Damage is : %f,  This Weapon Damage is : %f"), MainChar->EquippedWeapon->WeaponDamage, WeaponDamage);

			/* OnOverlap이 됐을때, 기존에 equipped weapon이 있다면,
			기존 Weapon의 Damage값과, 지금 Overlap된 Weapon의 Damage값을 띄워주고
			유저가 Damage를 보고 선택할 수 있게 하자.*/
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
		//Camera를 무시하도록 설정
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetSimulatePhysics(false); //Main Character와 Attach시켜주기 위해 physics를 끔.
		CollisionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		const USkeletalMeshSocket* RightWeaponSocket = MainChar->GetMesh()->GetSocketByName("hand_r_weapon");

		if (EquipedSound)
		{
			if (WeaponState == EWeaponState::EWS_Equipped)  //칼을 던지고 다시 받을때 재생되는 소리 따로 해둠.
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
			MainChar->SetEquippedWeapon(this); //Main의 SetEquipped Weapon 호출.
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

			FVector WeaponLocation = FMath::Lerp(CurWeaponLocation, Destination, AlphaTime); //AlphaTime때의 위치를 나타낸다.
			SetActorLocation(WeaponLocation);  //구한 위치를 계속해서 업데이트 해준다.

			FRotator WeaponRotation = GetActorRotation();
			WeaponRotation.Roll += GetWorld()->GetDeltaSeconds() * 2000.f; //2000속도의 속도로 회전하게 한다.
			FRotator WeaponRolling = FRotator(90.f, 0.f, WeaponRotation.Roll);  //vertical로 수평하게 하고 회전하게 한다.
			SetActorRotation(WeaponRolling);

			if (AlphaTime >= 1.f) //AlphaTime이 1일때 끝남.
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
			AMainCharacter* OwnerMain = Cast<AMainCharacter>(GetWeaponOwner()); //가해자가 Player
			AEnemy* OwnerEnemy = Cast<AEnemy>(GetWeaponOwner()); //가해자가 Enemy

			if (OwnerMain) //가해자가 Player일때(이 무기의 소유자가 Player일때)
			{
				AEnemy* Enemy = Cast<AEnemy>(OtherActor); //피해자
				if (Enemy)
				{
					UE_LOG(LogTemp, Warning, TEXT("Weapon::Overlap Actor is Enemy"));
					if (Enemy->HitParticle)
					{
						FVector HitLocation = SweepResult.ImpactPoint; //지금 안됨. 
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