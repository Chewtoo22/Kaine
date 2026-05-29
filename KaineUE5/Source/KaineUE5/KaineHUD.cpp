#include "KaineHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void AKaineHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas || !GEngine)
	{
		return;
	}

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float Pulse = 0.5f + 0.5f * FMath::Sin(Time * 4.5f);
	const FLinearColor Cyan(0.24f, 0.95f, 0.86f, 1.0f);
	const FLinearColor Amber(1.0f, 0.62f, 0.22f, 1.0f);
	const FLinearColor Violet(0.60f, 0.45f, 1.0f, 1.0f);
	const FLinearColor WhiteSoft(0.86f, 0.92f, 0.92f, 1.0f);
	const FLinearColor Muted(0.55f, 0.66f, 0.68f, 1.0f);
	const FLinearColor Glass(0.012f, 0.020f, 0.024f, 0.58f);

	DrawFrame(28.0f, 26.0f, W - 56.0f, H - 52.0f, FLinearColor(0.24f, 0.95f, 0.86f, 0.26f));
	DrawLine(W * 0.5f - 48.0f, H * 0.5f, W * 0.5f - 16.0f, H * 0.5f, FLinearColor(0.24f, 0.95f, 0.86f, 0.46f), 1.4f);
	DrawLine(W * 0.5f + 16.0f, H * 0.5f, W * 0.5f + 48.0f, H * 0.5f, FLinearColor(0.24f, 0.95f, 0.86f, 0.46f), 1.4f);
	DrawLine(W * 0.5f, H * 0.5f - 48.0f, W * 0.5f, H * 0.5f - 16.0f, FLinearColor(0.24f, 0.95f, 0.86f, 0.46f), 1.4f);
	DrawLine(W * 0.5f, H * 0.5f + 16.0f, W * 0.5f, H * 0.5f + 48.0f, FLinearColor(0.24f, 0.95f, 0.86f, 0.46f), 1.4f);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const float R = 30.0f + Index * 14.0f;
		DrawLine(W * 0.5f - R, H * 0.5f - R * 0.32f, W * 0.5f - R * 0.52f, H * 0.5f - R, Index % 2 == 0 ? FLinearColor(0.24f, 0.95f, 0.86f, 0.18f) : FLinearColor(1.0f, 0.62f, 0.22f, 0.22f), 1.0f);
		DrawLine(W * 0.5f + R, H * 0.5f + R * 0.32f, W * 0.5f + R * 0.52f, H * 0.5f + R, Index % 2 == 0 ? FLinearColor(0.24f, 0.95f, 0.86f, 0.18f) : FLinearColor(1.0f, 0.62f, 0.22f, 0.22f), 1.0f);
	}

	DrawPanel(34.0f, 30.0f, 316.0f, 86.0f, Glass);
	DrawLabel(TEXT("KAINE"), 52.0f, 42.0f, 1.20f, Cyan);
	DrawLabel(TEXT("POLISHED UE5 COMMAND CORE"), 54.0f, 77.0f, 0.60f, WhiteSoft);
	DrawLine(54.0f, 104.0f, 190.0f + Pulse * 72.0f, 104.0f, Amber, 2.0f);

	const float StripW = 392.0f;
	const float StripX = W * 0.5f - StripW * 0.5f;
	DrawPanel(StripX, 30.0f, StripW, 42.0f, FLinearColor(0.012f, 0.020f, 0.024f, 0.42f));
	const TCHAR* Tabs[] = { TEXT("CORE 98"), TEXT("MEM HOT"), TEXT("TOOLS IDLE"), TEXT("GUARD ON") };
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const float X = StripX + 14.0f + Index * 94.0f;
		DrawLine(X, 64.0f, X + 70.0f, 64.0f, Index == 3 ? Amber : (Index == 1 ? Violet : Cyan), Index == 0 ? 2.2f : 1.35f);
		DrawLabel(Tabs[Index], X, 42.0f, 0.52f, Index == 3 ? Amber : WhiteSoft);
	}

	DrawPanel(W - 382.0f, 30.0f, 348.0f, 108.0f, Glass);
	DrawLabel(TEXT("COMMAND LAB"), W - 362.0f, 46.0f, 0.80f, Amber);
	DrawLabel(TEXT("Holographic UI rebuilt as authored scene geometry"), W - 362.0f, 74.0f, 0.54f, WhiteSoft);
	DrawLabel(TEXT("WASD / QE / MOUSE"), W - 362.0f, 96.0f, 0.52f, Muted);
	DrawLine(W - 126.0f, 51.0f, W - 58.0f, 51.0f, Cyan, 2.0f);
	DrawLine(W - 126.0f, 61.0f, W - 82.0f + Pulse * 26.0f, 61.0f, Violet, 1.6f);

	const float WaveX = 44.0f;
	const float WaveY = H - 70.0f;
	DrawPanel(WaveX - 10.0f, WaveY - 74.0f, 236.0f, 58.0f, FLinearColor(0.012f, 0.020f, 0.024f, 0.38f));
	DrawLabel(TEXT("SPEECH ENVELOPE"), WaveX, WaveY - 62.0f, 0.56f, Cyan);
	for (int32 Index = 0; Index < 26; ++Index)
	{
		const float BarH = 7.0f + 26.0f * (0.5f + 0.5f * FMath::Sin(Time * 5.4f + Index * 0.72f));
		const FLinearColor BarColor = Index % 5 == 0 ? Amber : FLinearColor(0.24f, 0.95f, 0.86f, 0.55f + Pulse * 0.25f);
		DrawPanel(WaveX + Index * 7.4f, WaveY - BarH - 19.0f, 3.6f, BarH, BarColor);
	}

	const float CardX = W - 316.0f;
	const float CardY = H - 154.0f;
	DrawPanel(CardX, CardY, 282.0f, 104.0f, FLinearColor(0.012f, 0.020f, 0.024f, 0.42f));
	DrawLabel(TEXT("RUN STATE"), CardX + 18.0f, CardY + 16.0f, 0.58f, Cyan);
	DrawLabel(TEXT("VISUAL LAYER"), CardX + 18.0f, CardY + 42.0f, 0.48f, Muted);
	DrawLine(CardX + 110.0f, CardY + 51.0f, CardX + 242.0f, CardY + 51.0f, FLinearColor(0.24f, 0.95f, 0.86f, 0.18f), 5.0f);
	DrawLine(CardX + 110.0f, CardY + 51.0f, CardX + 202.0f + Pulse * 30.0f, CardY + 51.0f, Cyan, 3.0f);
	DrawLabel(TEXT("SAFETY GATES"), CardX + 18.0f, CardY + 68.0f, 0.48f, Muted);
	DrawLine(CardX + 110.0f, CardY + 77.0f, CardX + 242.0f, CardY + 77.0f, FLinearColor(1.0f, 0.62f, 0.22f, 0.18f), 5.0f);
	DrawLine(CardX + 110.0f, CardY + 77.0f, CardX + 228.0f, CardY + 77.0f, Amber, 3.0f);

	for (int32 Index = 0; Index < 10; ++Index)
	{
		const float X = W - 170.0f + FMath::Cos(Time * 0.8f + Index * 0.63f) * (28.0f + Index * 3.6f);
		const float Y = H - 260.0f + FMath::Sin(Time * 0.8f + Index * 0.63f) * (16.0f + Index * 2.1f);
		DrawPanel(X, Y, 4.0f, 4.0f, Index % 3 == 0 ? Amber : Cyan);
		if (Index > 0)
		{
			const float LastX = W - 170.0f + FMath::Cos(Time * 0.8f + (Index - 1) * 0.63f) * (28.0f + (Index - 1) * 3.6f);
			const float LastY = H - 260.0f + FMath::Sin(Time * 0.8f + (Index - 1) * 0.63f) * (16.0f + (Index - 1) * 2.1f);
			DrawLine(LastX + 2.0f, LastY + 2.0f, X + 2.0f, Y + 2.0f, FLinearColor(0.24f, 0.95f, 0.86f, 0.16f), 1.0f);
		}
	}
}

void AKaineHUD::DrawPanel(float X, float Y, float W, float H, const FLinearColor& Color) const
{
	FCanvasTileItem Tile(FVector2D(X, Y), FVector2D(W, H), Color);
	Tile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Tile);

	FCanvasTileItem Top(FVector2D(X, Y), FVector2D(W, 1.5f), FLinearColor(0.24f, 0.95f, 0.86f, 0.42f));
	Top.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Top);

	FCanvasTileItem Left(FVector2D(X, Y), FVector2D(1.5f, H), FLinearColor(0.24f, 0.95f, 0.86f, 0.22f));
	Left.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Left);

	FCanvasTileItem Bottom(FVector2D(X, Y + H - 1.5f), FVector2D(W, 1.5f), FLinearColor(1.0f, 0.62f, 0.22f, 0.18f));
	Bottom.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Bottom);
}

void AKaineHUD::DrawLabel(const FString& Text, float X, float Y, float Scale, const FLinearColor& Color) const
{
	FCanvasTextItem Item(FVector2D(X, Y), FText::FromString(Text), GEngine->GetSmallFont(), Color);
	Item.Scale = FVector2D(Scale, Scale);
	Item.EnableShadow(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f));
	Canvas->DrawItem(Item);
}

void AKaineHUD::DrawLine(float X1, float Y1, float X2, float Y2, const FLinearColor& Color, float Thickness) const
{
	FCanvasLineItem Line(FVector2D(X1, Y1), FVector2D(X2, Y2));
	Line.SetColor(Color);
	Line.LineThickness = Thickness;
	Canvas->DrawItem(Line);
}

void AKaineHUD::DrawFrame(float X, float Y, float W, float H, const FLinearColor& Color) const
{
	const float Corner = 74.0f;
	DrawLine(X, Y, X + Corner, Y, Color, 1.4f);
	DrawLine(X, Y, X, Y + Corner, Color, 1.4f);
	DrawLine(X + W, Y, X + W - Corner, Y, Color, 1.4f);
	DrawLine(X + W, Y, X + W, Y + Corner, Color, 1.4f);
	DrawLine(X, Y + H, X + Corner, Y + H, Color, 1.4f);
	DrawLine(X, Y + H, X, Y + H - Corner, Color, 1.4f);
	DrawLine(X + W, Y + H, X + W - Corner, Y + H, Color, 1.4f);
	DrawLine(X + W, Y + H, X + W, Y + H - Corner, Color, 1.4f);
}
