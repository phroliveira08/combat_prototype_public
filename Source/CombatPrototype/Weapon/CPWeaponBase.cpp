// Fill out your copyright notice in the Description page of Project Settings.


#include "CPWeaponBase.h"

#include "CombatPrototype/Character/CPCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACPWeaponBase::ACPWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>("WeaponMesh");
	WeaponMesh->SetupAttachment(RootComponent);

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
	CapsuleComp->SetupAttachment(WeaponMesh);
}

void ACPWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACPWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTraceActivated)
	{
		const FHitResult& HitInfo = ExecTraceHit();
		
		if (ACPCharacter* HitCharacter = Cast<ACPCharacter>(HitInfo.GetActor()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitInfo.GetActor()->GetName());
			
			HitCharacter->TakeHit(OwnerCharacter->GetCurrentAttack(), HitInfo);
			bTraceActivated = false;
		}
	}
}

void ACPWeaponBase::SetSocketWeaponAttached(const ACharacter* CharacterTarget, const ECPCombatPose CharacterPose) const
{
	const FName SocketName = GetSocketName(CharacterPose);
	
	const FAttachmentTransformRules AttachmentRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		true
		);
	
	WeaponMesh->AttachToComponent(CharacterTarget->GetMesh(), AttachmentRules, SocketName);
}

FName ACPWeaponBase::GetSocketName(const ECPCombatPose CharacterPose) const
{
	switch (CharacterPose)
	{
		case ECPCombatPose::None:
			return HolsterSocketName;
		case ECPCombatPose::LeftPose:
			return LeftHandSocketName;
		case ECPCombatPose::RightPose:
			return RightHandSocketName;
		default:
			return HolsterSocketName;
	}
}

void ACPWeaponBase::SetTraceStatus(const bool Active)
{
	bTraceActivated = Active;
}

FHitResult ACPWeaponBase::ExecTraceHit(const float MultiplierRadius) const
{
	FHitResult HitResult;

	const FVector StartLocation = CapsuleComp->GetComponentLocation();
	const FVector EndLocation = StartLocation;
	const FQuat CapsuleQuatRotation = CapsuleComp->GetComponentQuat();

	const float CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius() * MultiplierRadius;
	const float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();

	constexpr ECollisionChannel TraceChannel = ECC_WorldDynamic;

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(OwnerCharacter);

	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		CapsuleQuatRotation,
		TraceChannel,
		FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
		QueryParams
	);
		
	if (bTraceDebug)
	{
		const FColor Color = bHit ? FColor::Green : FColor::Red;
		DrawDebugCapsule(GetWorld(), (StartLocation + EndLocation) / 2, CapsuleHalfHeight, CapsuleRadius, CapsuleQuatRotation, Color, false, 5.0f);
	}

	return HitResult;
}

