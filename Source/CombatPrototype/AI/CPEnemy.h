// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatPrototype/Character/CPCharacter.h"
#include "CPEnemy.generated.h"

UCLASS()
class COMBATPROTOTYPE_API ACPEnemy : public ACPCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACPEnemy();

public:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
};
