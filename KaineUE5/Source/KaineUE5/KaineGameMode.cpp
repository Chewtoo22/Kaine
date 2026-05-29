#include "KaineGameMode.h"

#include "Engine/World.h"
#include "KaineCameraPawn.h"
#include "KaineCommandLab.h"
#include "KaineHUD.h"
#include "KaineUE5.h"

AKaineGameMode::AKaineGameMode()
{
	DefaultPawnClass = AKaineCameraPawn::StaticClass();
	HUDClass = AKaineHUD::StaticClass();
}

void AKaineGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("KainePhotorealCommandLab");
		World->SpawnActor<AKaineCommandLab>(AKaineCommandLab::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		UE_LOG(LogKaineUE5, Display, TEXT("Kaine UE5 photoreal command lab active: scene=1 pawn=1 hud=1 renderer=Lumen/VSM"));
	}
}
