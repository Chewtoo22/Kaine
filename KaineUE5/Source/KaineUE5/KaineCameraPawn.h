#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "KaineCameraPawn.generated.h"

class UCameraComponent;
class USceneComponent;

UCLASS()
class KAINEUE5_API AKaineCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AKaineCameraPawn();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	float MoveSpeed = 620.0f;
	float LookSpeed = 0.12f;
	float PitchLimit = 72.0f;
};
