// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ECPTutorialStep : uint8
{
	CombatMode UMETA(DisplayName = "Combat mode"),
	RightPose UMETA(DisplayName = "Right pose"),
	LeftPose UMETA(DisplayName = "Left pose"),
	TopPose UMETA(DisplayName = "Top pose"),
	MoveForward UMETA(DisplayName = "MoveForward"),
	MoveBackward UMETA(DisplayName = "MoveBackward"),
	MoveLeft UMETA(DisplayName = "MoveLeft"),
	MoveRight UMETA(DisplayName = "MoveRight"),
	LightAttack UMETA(DisplayName = "LightAttack"),
	Block UMETA(DisplayName = "Block"),
	Parry UMETA(DisplayName = "Parry"),
};