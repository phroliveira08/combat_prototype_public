// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatPrototype/Enums/CPCombatPose.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Actor.h"
#include "CPWeaponBase.generated.h"

class ACPCharacter;
class UCapsuleComponent;

UCLASS()
class COMBATPROTOTYPE_API ACPWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	/** Components Area **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh)
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh)
	UCapsuleComponent* CapsuleComp;
	
	/** Config Area **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	FName LeftHandSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	FName RightHandSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	FName HolsterSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	bool bTraceDebug;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	UParticleSystem* BloodEffect;

	/** State Area **/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Config)
	bool bTraceActivated;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Config)
	ACPCharacter* OwnerCharacter;

public:
	// Sets default values for this actor's properties
	ACPWeaponBase();

protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetSocketWeaponAttached(const ACharacter* CharacterTarget, const ECPCombatPose CharacterPose) const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	FName GetSocketName(ECPCombatPose CharacterPose) const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetTraceStatus(const bool Active);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	FHitResult ExecTraceHit(const float MultiplierRadius = 1.f) const;
};
