#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KaineGameMode.generated.h"

UCLASS()
class KAINEUE5_API AKaineGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AKaineGameMode();

	virtual void BeginPlay() override;
};
