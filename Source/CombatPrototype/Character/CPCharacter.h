// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatPrototype/Combat/CombatTypes.h"
#include "CombatPrototype/Enums/CPAttackType.h"
#include "GameFramework/Character.h"
#include "CombatPrototype/Enums/CPCombatPose.h"
#include "CombatPrototype/Enums/CPTutorialStep.h"
#include "CombatPrototype/Weapon/CPWeaponBase.h"
#include "Logging/LogMacros.h"
#include "CPCharacter.generated.h"

class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogCPCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatModeChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatPoseChange, ECPCombatPose, NewCombatPose, ECPCombatPose, OldCombatPose);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParryMovementChange, bool, bStarting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttacking);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialStepComplete, ECPTutorialStep, TutorialStepID);

UCLASS(config=Game)
class COMBATPROTOTYPE_API ACPCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	/** Config **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	float NormalMaxWalkSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	float CombatMaxWalkSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	bool StartCombatMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config, meta = (EditCondition = "StartCombatMode"))
	ECPCombatPose StartCombatPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	UAttackListAsset* AttackList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	TSubclassOf<ACPWeaponBase> WeaponToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	float DistanceToParry = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	bool bTraceToDebugParry = false;

	/** State **/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	bool IsCombatMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	ECPCombatPose CurrentCombatPose;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	ECPCombatPose PendingCombatPose;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	float CurrentStamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	bool bMovingForward;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	bool bMovingBackward;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	bool bMovingRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	bool bMovingLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	bool bCanAttackAgain = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	UAttackDataAsset* CurrentAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	UAttackDataAsset* PendingAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	TArray<UAttackDataAsset*> PreviousAttacks;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	ACPWeaponBase* CurrentWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	UAnimMontage* LastHitMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	bool bRunningHitMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	ACPCharacter* Target;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	ACPCharacter* Attacker;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	bool bParryChance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	bool bParryStunned;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = State)
	bool bBlocking;
	
public:
	ACPCharacter();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCombatModeChange OnCombatModeChange;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCombatPoseChange OnCombatPoseChange;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnParryMovementChange OnParryMovementChange;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAttacking OnAttacking;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTutorialStepComplete OnTutorialStepComplete;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	ECPCombatPose GetCombatPose() const { return CurrentCombatPose; };

	UFUNCTION(BlueprintCallable)
	void SetCombatPose(ECPCombatPose NewCombatPose);

	UFUNCTION(BlueprintCallable)
	bool GetIsCombatMode() const { return IsCombatMode; };

	UFUNCTION(BlueprintCallable)
	void SetIsCombatMode(bool NewValue);

	UFUNCTION(BlueprintCallable)
	ACPWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; };

	UFUNCTION(BlueprintCallable)
	UAttackDataAsset* GetCurrentAttack() const { return CurrentAttack; };

	UFUNCTION(BlueprintCallable)
	void TakeHit(UAttackDataAsset* AttackData, const FHitResult& HitInfo);

protected:
	UFUNCTION(BlueprintCallable)
	void TryAttack(ECPCombatPose CombatPose, ECPAttackType AttackType);

	UFUNCTION(BlueprintCallable)
	UAttackDataAsset* SearchAttack(ECPCombatPose CombatPose, ECPAttackType AttackType);

	UFUNCTION(BlueprintCallable)
	void BlockAttack(const bool bParry);

private:
	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void OnMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);
	
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
