// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ECPCombatPose : uint8
{
	None UMETA(DisplayName = "None"),
    LeftPose UMETA(DisplayName = "LeftPose"),
	RightPose UMETA(DisplayName = "RightPose"),
	TopPose UMETA(DisplayName = "TopPose")
};
