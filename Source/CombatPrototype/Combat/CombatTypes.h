// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatPrototype/Enums/CPAttackType.h"
#include "CombatPrototype/Enums/CPCombatPose.h"

#include "CombatTypes.generated.h"


class UAttackDataAsset;

USTRUCT(BlueprintType)
struct FAttackDependencies
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dependencies")
	TArray<UAttackDataAsset*> Attacks;
};

UCLASS(BlueprintType)
class UAttackDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack")
	ECPAttackType AttackType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack")
	ECPCombatPose CombatPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack")
	TArray<FAttackDependencies> Dependencies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	UAnimMontage* Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target | Animation")
	UAnimMontage* ParryMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target | Animation")
	UAnimMontage* RecoverParryMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target | Animation")
	UAnimMontage* DamageMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target | Animation")
	UAnimMontage* KnockdownMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float StaminaReduction;
};

UCLASS(BlueprintType)
class UAttackListAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack List")
	TArray<UAttackDataAsset*> Attacks;
};
