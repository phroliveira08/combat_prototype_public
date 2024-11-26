// Fill out your copyright notice in the Description page of Project Settings.


#include "CPCharacter.h"

#include "CollisionDebugDrawingPublic.h"
#include "KismetTraceUtils.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogCPCharacter);

ACPCharacter::ACPCharacter()
{
	GetCharacterMovement()->MaxWalkSpeed = NormalMaxWalkSpeed;
	
	IsCombatMode = StartCombatMode;

	if (IsCombatMode)
	{
		SetCombatPose(StartCombatPose);
	}
	else
	{
		SetCombatPose(ECPCombatPose::None);
	}
}

void ACPCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	if (WeaponToSpawn)
	{
		CurrentWeapon = GetWorld()->SpawnActor<ACPWeaponBase>(WeaponToSpawn, GetActorTransform(), SpawnParams);
		CurrentWeapon->SetSocketWeaponAttached(this, CurrentCombatPose);
		CurrentWeapon->OwnerCharacter = this;
	}
	
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ACPCharacter::OnMontageNotifyBegin);
		AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &ACPCharacter::OnMontageNotifyEnd);
		AnimInstance->OnMontageBlendingOut.AddDynamic(this, &ACPCharacter::OnMontageBlendingOut);
		AnimInstance->OnMontageEnded.AddDynamic(this, &ACPCharacter::OnMontageEnded);
	}
}

void ACPCharacter::SetCombatPose(const ECPCombatPose NewCombatPose)
{
	if (NewCombatPose == CurrentCombatPose) return;
	
	if (OnCombatPoseChange.IsBound())
	{
		OnCombatPoseChange.Broadcast(NewCombatPose, CurrentCombatPose);
	}

	CurrentCombatPose = NewCombatPose;
}

void ACPCharacter::SetIsCombatMode(const bool NewValue)
{
	IsCombatMode = NewValue;
	
	if (IsCombatMode)
	{
		UE_LOG(LogCPCharacter, Display, TEXT("Combat mode enabled"));
		GetCharacterMovement()->MaxWalkSpeed = CombatMaxWalkSpeed;
	}
	else
	{
		UE_LOG(LogCPCharacter, Display, TEXT("Combat mode disabled"));
		GetCharacterMovement()->MaxWalkSpeed = NormalMaxWalkSpeed;
	}

	if (OnCombatModeChange.IsBound())
	{
		OnCombatModeChange.Broadcast();
	}
}

void ACPCharacter::TakeHit(UAttackDataAsset* AttackData, const FHitResult& HitInfo)
{
	if (AttackData == nullptr || bBlocking)
	{
		return;
	}
	
	UE_LOG(LogCPCharacter, Display, TEXT("Take hit"));

	bRunningHitMontage = true;
	
	if (CurrentAttack)
	{
		StopAnimMontage(CurrentAttack->Montage);
	}

	CurrentHealth = CurrentHealth - AttackData->Damage;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CurrentWeapon->BloodEffect, HitInfo.Location);
	
	if (CurrentHealth <= 0)
	{
		PlayAnimMontage(AttackData->KnockdownMontage);
		LastHitMontage = AttackData->KnockdownMontage;
		CurrentHealth = MaxHealth;
	}
	else
	{
		PlayAnimMontage(AttackData->DamageMontage);
		LastHitMontage = AttackData->DamageMontage;
	}
}

void ACPCharacter::TryAttack(ECPCombatPose CombatPose, ECPAttackType AttackType)
{
	if (!IsCombatMode || !bCanAttackAgain || bRunningHitMontage || bParryStunned) return;

	if (CurrentAttack == nullptr && AttackType == ECPAttackType::Light && bParryChance)
	{
		if (Attacker && CurrentCombatPose == Attacker->GetCombatPose())
		{
			bBlocking = true;
			BlockAttack(true);
			return;
		}
	}

	if (AttackList == nullptr)
	{
		UE_LOG(LogCPCharacter, Error, TEXT("Error: CPCharacter without attack list defined"));
		return;
	}

	UAttackDataAsset* SelectedAttack = SearchAttack(CombatPose, AttackType);

	if (SelectedAttack == nullptr)
	{
		UE_LOG(LogCPCharacter, Display, TEXT("Not found any attack to this condition"));
		return;
	}

	bCanAttackAgain = false;
	
	if (CurrentAttack)
	{
		UE_LOG(LogCPCharacter, Display, TEXT("Add Pending Attack"));
		PendingAttack = SelectedAttack;
		return;
	}

	CurrentAttack = SelectedAttack;
	PlayAnimMontage(SelectedAttack->Montage);

	OnTutorialStepComplete.Broadcast(ECPTutorialStep::LightAttack);

	if (OnAttacking.IsBound())
	{
		OnAttacking.Broadcast();
	}
	
	UE_LOG(LogCPCharacter, Display, TEXT("Playing Normal Attack"));
}

UAttackDataAsset* ACPCharacter::SearchAttack(ECPCombatPose CombatPose, ECPAttackType AttackType)
{
	UE_LOG(LogCPCharacter, Display, TEXT("Searching attacks"));

	TArray<UAttackDataAsset*> AttacksToCompare = this->PreviousAttacks;
	
	if(CurrentAttack)
	{
		AttacksToCompare.Push(CurrentAttack);
	}
	
	for ( UAttackDataAsset* AttackData : AttackList->Attacks)
	{
		if (AttackData->CombatPose == CombatPose && AttackData->AttackType == AttackType)
		{
			if(CurrentAttack == nullptr && AttackData->Dependencies.Num() == 0)
			{
				return AttackData;
			}
			
			for (auto& [Value] : AttackData->Dependencies)
			{
				if (Value == AttacksToCompare)
				{
					return AttackData;
				}
			}
		}
	}
	return nullptr;
}

void ACPCharacter::BlockAttack(const bool bParry)
{
	bCanAttackAgain = false;
			
	const float CurrentPosition = Attacker->GetRootMotionAnimMontageInstance()->GetPosition();
	const float AttackHitMoment = Attacker->GetCurrentMontage()->GetSectionTimeLeftFromPos(0);
	const float ParryHitMoment = Attacker->CurrentAttack->ParryMontage->GetSectionTimeLeftFromPos(0);
	
	const float PlayBlockAnimTime = AttackHitMoment - ParryHitMoment - CurrentPosition;
	
	if (OnParryMovementChange.IsBound() && bParry)
	{
		OnParryMovementChange.Broadcast(true);
	}

	if (PlayBlockAnimTime < 0.f)
	{
		PlayAnimMontage(Attacker->CurrentAttack->ParryMontage);
	}
	else
	{
		FTimerHandle TimerHandlePlayBlock;
		GetWorldTimerManager().SetTimer(TimerHandlePlayBlock, FTimerDelegate::CreateLambda([this]()
		{
			PlayAnimMontage(Attacker->CurrentAttack->ParryMontage);
		}), PlayBlockAnimTime, false);
	}

	FTimerHandle TimerHandleReturnMovement;
	
	const float StopAttackTime = AttackHitMoment - CurrentPosition;
	float ReturnMovementTime = StopAttackTime;
	
	if (bParry)
	{
		FTimerHandle TimerHandleStopAttack;
		
		ReturnMovementTime += 0.15f;
		
		GetWorldTimerManager().SetTimer(TimerHandleStopAttack, FTimerDelegate::CreateLambda([this]()
		{
			FAnimMontageInstance* AttackerMontageInstance = Attacker->GetRootMotionAnimMontageInstance();
			FAnimMontageInstance* SelfMontageInstance = GetRootMotionAnimMontageInstance();
			AttackerMontageInstance->Pause();
			SelfMontageInstance->Pause();
			
		}), StopAttackTime, false);

		GetWorldTimerManager().SetTimer(TimerHandleReturnMovement, FTimerDelegate::CreateLambda([this]()
		{
			if (OnParryMovementChange.IsBound())
			{
				OnParryMovementChange.Broadcast(false);
			}
	
			GetRootMotionAnimMontageInstance()->Play();
			Attacker->bParryStunned = true;
			Attacker->PlayAnimMontage(Attacker->CurrentAttack->RecoverParryMontage);
			bBlocking = false;
		}), ReturnMovementTime, false);

		OnTutorialStepComplete.Broadcast(ECPTutorialStep::Parry);
	}
	else
	{
		GetWorldTimerManager().SetTimer(TimerHandleReturnMovement, FTimerDelegate::CreateLambda([this]()
		{
			Attacker->StopAnimMontage(Attacker->CurrentAttack->Montage);
		}), ReturnMovementTime, false);
		
		OnTutorialStepComplete.Broadcast(ECPTutorialStep::Block);
	}
}

void ACPCharacter::OnMontageNotifyBegin(FName NotifyName,
                                        const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName == "ComboChance")
	{
		UE_LOG(LogCPCharacter, Display, TEXT("Start Combo Chance"));
		bCanAttackAgain = true;
	}

	if (NotifyName == "ParryChance")
	{
		UE_LOG(LogCPCharacter, Display, TEXT("Start ParryChance"));
		
		FHitResult HitResult;

		const FVector StartLocation = GetActorLocation();
		const FVector EndLocation = StartLocation + GetActorForwardVector() * DistanceToParry;

		constexpr ECollisionChannel TraceChannel = ECC_WorldDynamic;

		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = true;
		QueryParams.AddIgnoredActor(this);

		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartLocation,
			EndLocation,
			TraceChannel,
			QueryParams
		);

#if ENABLE_DRAW_DEBUG
		
		if (bTraceToDebugParry)
		{
			DrawDebugLineTraceSingle(GetWorld(), StartLocation, EndLocation, EDrawDebugTrace::ForDuration, bHit, HitResult, FLinearColor::Red, FLinearColor::Green, 5.0f);
		}
#endif

		if (bHit)
		{
			if (ACPCharacter* NewTarget = Cast<ACPCharacter>(HitResult.GetActor()))
			{
				Target = NewTarget;
				
				UE_LOG(LogCPCharacter, Display, TEXT("Add Target %s"), *Target->GetName());
				Target->Attacker = this;
				Target->bParryChance = true;
			}
		}
	}
}

void ACPCharacter::OnMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName == "ComboChance")
	{
		UE_LOG(LogCPCharacter, Display, TEXT("End Combo Chance"));
		bCanAttackAgain = false;
	}

	if (NotifyName == "ParryChance")
	{
		UE_LOG(LogCPCharacter, Display, TEXT("End ParryChance"));

		if (Target)
		{
			UE_LOG(LogCPCharacter, Display, TEXT("Remove Target %s"), *Target->GetName());
			Target->bParryChance = false;

			if (CurrentAttack->CombatPose == Target->GetCombatPose() && Target->bCanAttackAgain)
			{
				Target->BlockAttack(false);
			}
		}
	}
}

void ACPCharacter::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (LastHitMontage == Montage)
	{
		bRunningHitMontage = false;
		return;
	}

	if (Target)
	{
		UE_LOG(LogCPCharacter, Display, TEXT("Remove Target %s"), *Target->GetName());
		Target->Attacker = nullptr;
		Target = nullptr;
	}
	
	if (CurrentAttack && CurrentAttack->Montage == Montage)
	{
		if (PendingAttack)
		{
			PreviousAttacks.Add(CurrentAttack);
			CurrentAttack = PendingAttack;
			PendingAttack = nullptr;
			PlayAnimMontage(CurrentAttack->Montage);
			UE_LOG(LogCPCharacter, Display, TEXT("Playing Pending Attack"));
		}
		else if(!bParryStunned)
		{
			CurrentAttack = nullptr;
		}
	}
}


void ACPCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (LastHitMontage == Montage)
	{
		LastHitMontage = nullptr;
		return;
	}

	if (CurrentAttack && CurrentAttack->RecoverParryMontage == Montage)
	{
		UE_LOG(LogCPCharacter, Display, TEXT("Remove Stunned"));
		bParryStunned = false;
		CurrentAttack = nullptr;
	}
	
	if (CurrentAttack == nullptr)
	{
		UE_LOG(LogCPCharacter, Display, TEXT("Cleaning Attack State"));
		bCanAttackAgain = true;
		PreviousAttacks.Empty();
	}
}









