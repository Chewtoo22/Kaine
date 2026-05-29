#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "KaineHUD.generated.h"

UCLASS()
class KAINEUE5_API AKaineHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawPanel(float X, float Y, float W, float H, const FLinearColor& Color) const;
	void DrawLabel(const FString& Text, float X, float Y, float Scale, const FLinearColor& Color) const;
	void DrawLine(float X1, float Y1, float X2, float Y2, const FLinearColor& Color, float Thickness = 1.0f) const;
	void DrawFrame(float X, float Y, float W, float H, const FLinearColor& Color) const;
};
