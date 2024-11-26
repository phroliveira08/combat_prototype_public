// Fill out your copyright notice in the Description page of Project Settings.


#include "CheckHitNotifyState.h"

#include "CombatPrototype/Character/CPCharacter.h"

void UCheckHitNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UE_LOG(LogTemp, Display, TEXT("NotifyBegin - CheckHit"));
	
	if (ACPCharacter* Owner = Cast<ACPCharacter>(MeshComp->GetOwner()))
	{
		Owner->GetCurrentWeapon()->SetTraceStatus(true);
	}
}

void UCheckHitNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	UE_LOG(LogTemp, Display, TEXT("NotifyEnd - CheckHit"));

	if (ACPCharacter* Owner = Cast<ACPCharacter>(MeshComp->GetOwner()))
	{
		Owner->GetCurrentWeapon()->SetTraceStatus(false);
	}
}
