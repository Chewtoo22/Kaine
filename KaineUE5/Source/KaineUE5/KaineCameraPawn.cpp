#include "KaineCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"

AKaineCameraPawn::AKaineCameraPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("KaineCamera"));
	Camera->SetupAttachment(SceneRoot);
	Camera->SetFieldOfView(46.0f);
	Camera->PostProcessSettings.bOverride_BloomIntensity = true;
	Camera->PostProcessSettings.BloomIntensity = 1.15f;
	Camera->PostProcessSettings.bOverride_VignetteIntensity = true;
	Camera->PostProcessSettings.VignetteIntensity = 0.38f;
	Camera->PostProcessSettings.bOverride_DepthOfFieldFstop = true;
	Camera->PostProcessSettings.DepthOfFieldFstop = 2.8f;
	Camera->PostProcessBlendWeight = 1.0f;
}

void AKaineCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	const FVector CameraLocation(-620.0f, -780.0f, 318.0f);
	const FVector FocusPoint(-36.0f, -42.0f, 214.0f);
	SetActorLocation(CameraLocation);
	SetActorRotation((FocusPoint - CameraLocation).Rotation());
}

void AKaineCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AKaineCameraPawn::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AKaineCameraPawn::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("MoveUp"), this, &AKaineCameraPawn::MoveUp);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AKaineCameraPawn::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AKaineCameraPawn::LookUp);
}

void AKaineCameraPawn::MoveForward(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddActorWorldOffset(GetActorForwardVector() * Value * MoveSpeed * GetWorld()->GetDeltaSeconds(), true);
	}
}

void AKaineCameraPawn::MoveRight(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddActorWorldOffset(GetActorRightVector() * Value * MoveSpeed * GetWorld()->GetDeltaSeconds(), true);
	}
}

void AKaineCameraPawn::MoveUp(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddActorWorldOffset(FVector::UpVector * Value * MoveSpeed * GetWorld()->GetDeltaSeconds(), true);
	}
}

void AKaineCameraPawn::Turn(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddActorWorldRotation(FRotator(0.0f, Value * LookSpeed, 0.0f));
	}
}

void AKaineCameraPawn::LookUp(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		const FRotator Current = GetActorRotation();
		const float NewPitch = FMath::ClampAngle(Current.Pitch + Value * LookSpeed, -PitchLimit, PitchLimit);
		SetActorRotation(FRotator(NewPitch, Current.Yaw, Current.Roll));
	}
}
