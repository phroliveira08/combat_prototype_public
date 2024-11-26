// Fill out your copyright notice in the Description page of Project Settings.


#include "CPCharacterPlayer.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"


ACPCharacterPlayer::ACPCharacterPlayer()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

void ACPCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentCameraOffset = NormalCameraOffset;
	TargetCameraOffset = CurrentCameraOffset;
	SpringArm->SocketOffset = CurrentCameraOffset;

	CurrentTargetArmLength = NormalTargetArmLength;
	TargetArmLength = CurrentTargetArmLength;
	SpringArm->TargetArmLength = CurrentTargetArmLength;

	CurrentCameraRotation = NormalCameraRotation;
	TargetCameraRotation = CurrentCameraRotation;
	Camera->SetRelativeRotation(CurrentCameraRotation);
	
	CurrentCameraFOV = NormalCameraFov;
	TargetCameraFOV = CurrentCameraFOV;
	Camera->FieldOfView = CurrentCameraFOV;

	OnParryMovementChange.AddDynamic(this, &ACPCharacterPlayer::ChanceCameraToParry);
}

void ACPCharacterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetCameraFOV != CurrentCameraFOV)
	{
		CurrentCameraFOV = FMath::FInterpTo(CurrentCameraFOV, TargetCameraFOV, DeltaTime, 10);
		Camera->FieldOfView = CurrentCameraFOV;
	}

	if (TargetArmLength != CurrentTargetArmLength)
	{
		CurrentTargetArmLength = FMath::FInterpTo(CurrentTargetArmLength, TargetArmLength, DeltaTime, 10);
		SpringArm->TargetArmLength = CurrentTargetArmLength;
	}
	
	if (TargetCameraOffset != CurrentCameraOffset)
	{
		CurrentCameraOffset = FMath::VInterpTo(CurrentCameraOffset, TargetCameraOffset, DeltaTime, 5);
		SpringArm->SocketOffset = CurrentCameraOffset;
	}
	else if (IsChangingPose)
	{
		IsChangingPose = false;
	}

	if (TargetCameraRotation != CurrentCameraRotation)
	{
		CurrentCameraRotation = FMath::RInterpTo(CurrentCameraRotation, TargetCameraRotation, DeltaTime, 10);
		Camera->SetRelativeRotation(CurrentCameraRotation);
	}
	
}

void ACPCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACPCharacterPlayer::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACPCharacterPlayer::Look);
		EnhancedInputComponent->BindAction(CombatModeAction, ETriggerEvent::Triggered, this, &ACPCharacterPlayer::CombatMode);
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &ACPCharacterPlayer::LightAttack);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &ACPCharacterPlayer::HeavyAttack);
	}
	else
	{
		UE_LOG(LogCPCharacter, Error, TEXT("Not find EnhancedInputComponent on SetupPlayerInputComponent"))
	}
}

void ACPCharacterPlayer::Move(const FInputActionValue& InputActionValue)
{
	FVector2d MovementVector = InputActionValue.Get<FVector2d>();

	if (Controller != nullptr)
	{
		//UE_LOG(LogCPCharacter, Display, TEXT("TECLADO - X: %f / Y: %f"), MovementVector.X, MovementVector.Y);

		if (MovementVector.Y > 0)
		{
			bMovingForward = true;
			bMovingBackward = false;
			OnTutorialStepComplete.Broadcast(ECPTutorialStep::MoveForward);
		}
		else if (MovementVector.Y < 0)
		{
			bMovingForward = false;
			bMovingBackward = true;
			OnTutorialStepComplete.Broadcast(ECPTutorialStep::MoveBackward);
		}
		else
		{
			bMovingBackward = false;
			bMovingForward = false;
		}

		if (MovementVector.X > 0)
		{
			bMovingRight = true;
			bMovingLeft = false;
			OnTutorialStepComplete.Broadcast(ECPTutorialStep::MoveRight);
		}
		else if (MovementVector.X < 0)
		{
			bMovingRight = false;
			bMovingLeft = true;
			OnTutorialStepComplete.Broadcast(ECPTutorialStep::MoveLeft);
		}
		else
		{
			bMovingRight = false;
			bMovingLeft = false;
		}
	}
}

void ACPCharacterPlayer::Look(const FInputActionValue& InputActionValue)
{
	const FVector2d LookVector = InputActionValue.Get<FVector2d>();
	
	if (Controller != nullptr)
	{
		if (!IsCombatMode)
		{
			AddControllerYawInput(LookVector.X);
			AddControllerPitchInput(LookVector.Y);
		}
		else
		{
			//UE_LOG(LogCPCharacter, Display, TEXT("MOUSE - X: %f / Y: %f"), LookVector.X, LookVector.Y);

			if (LookVector.Y < -VerticalThreshold && LookVector.X < HorizontalThreshold)
			{
				SetCombatPose(ECPCombatPose::TopPose);
				IsChangingPose = true;
				OnTutorialStepComplete.Broadcast(ECPTutorialStep::TopPose);
			}
			else if (LookVector.X > HorizontalThreshold)
			{
				SetCombatPose(ECPCombatPose::RightPose);
				IsChangingPose = true;
				OnTutorialStepComplete.Broadcast(ECPTutorialStep::RightPose);
			}
			else if (LookVector.X < -HorizontalThreshold)
			{
				SetCombatPose(ECPCombatPose::LeftPose);
				IsChangingPose = true;
				OnTutorialStepComplete.Broadcast(ECPTutorialStep::LeftPose);
			}
		}
	}
}

void ACPCharacterPlayer::CombatMode(const FInputActionValue& InputActionValue)
{
	SetIsCombatMode(!IsCombatMode);

	OnTutorialStepComplete.Broadcast(ECPTutorialStep::CombatMode);
	
	if (IsCombatMode)
	{
		TargetCameraOffset = CombatCameraOffset;
		TargetCameraFOV = CombatCameraFov;
		TargetArmLength = CombatTargetArmLength;
		SetCombatPose(ECPCombatPose::RightPose);
	}
	else
	{
		TargetCameraOffset = NormalCameraOffset;
		TargetCameraFOV = NormalCameraFov;
		TargetArmLength = NormalTargetArmLength;
		SetCombatPose(ECPCombatPose::None);
	}
}

void ACPCharacterPlayer::LightAttack(const FInputActionValue& InputActionValue)
{
	TryAttack(CurrentCombatPose, ECPAttackType::Light);
}

void ACPCharacterPlayer::HeavyAttack(const FInputActionValue& ActionValue)
{
	TryAttack(CurrentCombatPose, ECPAttackType::Heavy);
}

void ACPCharacterPlayer::ChanceCameraToParry(const bool bStarting)
{
	if (bStarting)
	{
		TargetCameraRotation = ParryCameraRotation;
		TargetCameraOffset = ParryCameraOffset;
		TargetCameraFOV = ParryCameraFov;
	}
	else
	{
		TargetCameraRotation = NormalCameraRotation;
		TargetCameraOffset = CombatCameraOffset;
		TargetCameraFOV = CombatCameraFov;
	}
}


