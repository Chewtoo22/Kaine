#include "KaineCommandLab.h"

#include "Components/PointLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/RectLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "KaineUE5.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/RotationMatrix.h"
#include "ProceduralMeshComponent.h"

AKaineCommandLab::AKaineCommandLab()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	RootComponent = Root;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	CubeMesh = CubeFinder.Object;
	SphereMesh = SphereFinder.Object;
	CylinderMesh = CylinderFinder.Object;
	PlaneMesh = PlaneFinder.Object;
	BaseMaterial = MaterialFinder.Object;
}

void AKaineCommandLab::BeginPlay()
{
	Super::BeginPlay();

	BuildRoom();
	BuildKaineCore();
	BuildWorkstations();
	BuildAuthoredAssetLayer();
	BuildServerWall();
	BuildHolographicInterface();
	BuildPremiumUILayer();
	BuildLightingAndAtmosphere();

	UE_LOG(LogKaineUE5, Display, TEXT("Kaine polished UI scene spawned: ring_segments=%d panel_lights=%d hologram_panels=%d data_bars=%d authored_panels=%d premium_ui=%d"), RingSegments.Num(), PanelLights.Num(), HologramPanels.Num(), DataBars.Num(), AuthoredPanels.Num(), PremiumUIElements.Num());
}

void AKaineCommandLab::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float Pulse = 0.5f + 0.5f * FMath::Sin(Time * 4.1f);

	if (CoreSphere)
	{
		CoreSphere->AddLocalRotation(FRotator(0.0f, 16.0f * DeltaSeconds, 7.0f * DeltaSeconds));
		CoreSphere->SetRelativeScale3D(FVector(1.05f + Pulse * 0.05f));
	}

	for (int32 Index = 0; Index < RingSegments.Num(); ++Index)
	{
		if (UStaticMeshComponent* Segment = RingSegments[Index])
		{
			const float Direction = (Index % 2 == 0) ? 1.0f : -1.0f;
			Segment->AddLocalRotation(FRotator(0.0f, Direction * 18.0f * DeltaSeconds, Direction * 6.0f * DeltaSeconds));
		}
	}

	for (int32 Index = 0; Index < CoreLights.Num(); ++Index)
	{
		if (APointLight* LightActor = CoreLights[Index])
		{
			if (UPointLightComponent* Light = LightActor->PointLightComponent)
			{
				Light->SetIntensity(2200.0f + Pulse * 2800.0f + Index * 300.0f);
			}
		}
	}

	for (int32 Index = 0; Index < DataBars.Num(); ++Index)
	{
		if (UStaticMeshComponent* Bar = DataBars[Index])
		{
			const float Phase = Time * (2.0f + Index * 0.035f) + Index * 0.47f;
			const float Length = 0.12f + 0.34f * (0.5f + 0.5f * FMath::Sin(Phase));
			const FVector BaseScale = Bar->GetRelativeScale3D();
			Bar->SetRelativeScale3D(FVector(Length, BaseScale.Y, BaseScale.Z));
		}
	}
}

void AKaineCommandLab::BuildRoom()
{
	UMaterialInstanceDynamic* FloorMat = MakeMaterial(TEXT("WetGraphiteFloor"), FLinearColor(0.022f, 0.026f, 0.028f, 1.0f), 0.85f, 0.18f, 0.0f);
	UMaterialInstanceDynamic* WallMat = MakeMaterial(TEXT("DarkTitaniumWall"), FLinearColor(0.028f, 0.034f, 0.038f, 1.0f), 0.72f, 0.28f, 0.0f);
	UMaterialInstanceDynamic* GlassMat = MakeMaterial(TEXT("BlueBlackGlass"), FLinearColor(0.035f, 0.12f, 0.15f, 0.66f), 0.2f, 0.04f, 0.2f);
	UMaterialInstanceDynamic* EdgeMat = MakeMaterial(TEXT("WarmEdgeTrim"), FLinearColor(0.85f, 0.58f, 0.27f, 1.0f), 0.75f, 0.22f, 0.15f);

	AddMesh(TEXT("MirrorPolishedFloor"), CubeMesh, FVector(0.0f, 0.0f, -14.0f), FRotator::ZeroRotator, FVector(18.0f, 14.0f, 0.16f), FloorMat);
	AddMesh(TEXT("RearTitaniumWall"), CubeMesh, FVector(520.0f, 0.0f, 220.0f), FRotator::ZeroRotator, FVector(0.16f, 14.0f, 4.8f), WallMat);
	AddMesh(TEXT("LeftTitaniumWall"), CubeMesh, FVector(0.0f, 720.0f, 220.0f), FRotator::ZeroRotator, FVector(18.0f, 0.16f, 4.8f), WallMat);
	AddMesh(TEXT("RightTitaniumWall"), CubeMesh, FVector(0.0f, -720.0f, 220.0f), FRotator::ZeroRotator, FVector(18.0f, 0.16f, 4.8f), WallMat);
	AddMesh(TEXT("LowReflectiveCeiling"), CubeMesh, FVector(0.0f, 0.0f, 505.0f), FRotator::ZeroRotator, FVector(18.0f, 14.0f, 0.12f), WallMat);

	for (int32 Index = 0; Index < 8; ++Index)
	{
		const float Y = -560.0f + Index * 160.0f;
		AddMesh(FString::Printf(TEXT("RearGlassRib_%02d"), Index), CubeMesh, FVector(498.0f, Y, 210.0f), FRotator::ZeroRotator, FVector(0.08f, 0.24f, 3.6f), GlassMat);
		AddMesh(FString::Printf(TEXT("FloorBronzeGuide_%02d"), Index), CubeMesh, FVector(-360.0f + Index * 110.0f, -610.0f, -2.0f), FRotator::ZeroRotator, FVector(0.42f, 0.02f, 0.025f), EdgeMat);
		AddMesh(FString::Printf(TEXT("CeilingBronzeGuide_%02d"), Index), CubeMesh, FVector(-360.0f + Index * 110.0f, 610.0f, 490.0f), FRotator::ZeroRotator, FVector(0.42f, 0.02f, 0.025f), EdgeMat);
	}

	UMaterialInstanceDynamic* StepMat = MakeMaterial(TEXT("KaineFloorLightCuts"), FLinearColor(0.08f, 0.42f, 0.44f, 1.0f), 0.45f, 0.1f, 0.9f);
	for (int32 Index = 0; Index < 9; ++Index)
	{
		const float X = -500.0f + Index * 125.0f;
		AddMesh(FString::Printf(TEXT("FloorInterfaceCut_%02d"), Index), CubeMesh, FVector(X, 0.0f, -1.0f), FRotator::ZeroRotator, FVector(0.04f, 6.2f, 0.015f), StepMat);
	}
}

void AKaineCommandLab::BuildKaineCore()
{
	UMaterialInstanceDynamic* CoreMat = MakeMaterial(TEXT("KaineInnerCore"), FLinearColor(0.17f, 0.96f, 0.88f, 1.0f), 0.18f, 0.08f, 3.0f);
	UMaterialInstanceDynamic* RingMat = MakeMaterial(TEXT("KaineEnergyRing"), FLinearColor(0.21f, 0.86f, 0.98f, 1.0f), 0.2f, 0.05f, 2.4f);
	UMaterialInstanceDynamic* AmberMat = MakeMaterial(TEXT("KaineAmberData"), FLinearColor(1.0f, 0.58f, 0.16f, 1.0f), 0.3f, 0.08f, 1.7f);
	UMaterialInstanceDynamic* FacePlateMat = MakeMaterial(TEXT("KaineSmokedFacePlate"), FLinearColor(0.01f, 0.055f, 0.065f, 1.0f), 0.4f, 0.06f, 0.45f);

	CoreSphere = AddMesh(TEXT("KaineLivingCore"), SphereMesh, FVector(0.0f, 0.0f, 172.0f), FRotator::ZeroRotator, FVector(1.05f), CoreMat, true);
	AddMesh(TEXT("CorePedestal"), CylinderMesh, FVector(0.0f, 0.0f, 56.0f), FRotator::ZeroRotator, FVector(1.75f, 1.75f, 0.5f), MakeMaterial(TEXT("BrushedPedestal"), FLinearColor(0.05f, 0.058f, 0.06f, 1.0f), 0.95f, 0.18f, 0.0f));
	AddMesh(TEXT("CoreGlassPlinth"), CylinderMesh, FVector(0.0f, 0.0f, 92.0f), FRotator::ZeroRotator, FVector(2.15f, 2.15f, 0.06f), RingMat);

	const FVector FaceForward(-0.63f, -0.78f, 0.0f);
	const FVector FaceRight(FaceForward.Y, -FaceForward.X, 0.0f);
	const FVector FaceCenter = FVector(0.0f, 0.0f, 172.0f) + FaceForward * 92.0f;
	const FRotator FaceRot = FaceForward.Rotation();
	const FRotator EyeRot = FaceRight.Rotation();
	AddMesh(TEXT("KaineSmokedFaceShield"), CubeMesh, FaceCenter + FVector(0.0f, 0.0f, 2.0f), FaceRot, FVector(0.035f, 0.72f, 0.68f), FacePlateMat, true);
	AddMesh(TEXT("KaineLeftEyePlate"), CubeMesh, FaceCenter - FaceRight * 32.0f + FVector(0.0f, 0.0f, 32.0f), EyeRot, FVector(0.34f, 0.032f, 0.045f), RingMat, true);
	AddMesh(TEXT("KaineRightEyePlate"), CubeMesh, FaceCenter + FaceRight * 32.0f + FVector(0.0f, 0.0f, 32.0f), EyeRot, FVector(0.34f, 0.032f, 0.045f), RingMat, true);
	AddMesh(TEXT("KaineBrowCut"), CubeMesh, FaceCenter + FVector(0.0f, 0.0f, 52.0f), EyeRot, FVector(0.78f, 0.018f, 0.022f), AmberMat, true);

	for (int32 Index = 0; Index < 7; ++Index)
	{
		const float Offset = -27.0f + Index * 9.0f;
		const float Height = 0.055f + (Index % 3) * 0.018f;
		UStaticMeshComponent* SpeechBar = AddMesh(FString::Printf(TEXT("KaineFrontSpeechBar_%02d"), Index), CubeMesh, FaceCenter + FaceRight * Offset + FVector(0.0f, 0.0f, -24.0f), FRotator::ZeroRotator, FVector(0.028f, 0.018f, Height), AmberMat, true);
		DataBars.Add(SpeechBar);
	}

	BuildRing(150.0f, 32, FVector::ForwardVector, FVector::RightVector, FVector(0.0f, 0.0f, 172.0f), FLinearColor(0.21f, 0.86f, 0.98f, 1.0f));
	BuildRing(205.0f, 42, FVector::ForwardVector, FVector::UpVector, FVector(0.0f, 0.0f, 172.0f), FLinearColor(1.0f, 0.58f, 0.16f, 1.0f));
	BuildRing(250.0f, 54, FVector::RightVector, FVector::UpVector, FVector(0.0f, 0.0f, 172.0f), FLinearColor(0.62f, 0.46f, 1.0f, 1.0f));

	for (int32 Index = 0; Index < 12; ++Index)
	{
		const float Angle = FMath::DegreesToRadians(Index * 30.0f);
		const FVector Loc(FMath::Cos(Angle) * 74.0f, FMath::Sin(Angle) * 74.0f, 104.0f);
		const FRotator Rot(0.0f, FMath::RadiansToDegrees(Angle), 0.0f);
		AddMesh(FString::Printf(TEXT("AmberMouthBar_%02d"), Index), CubeMesh, Loc, Rot, FVector(0.1f, 0.018f, 0.08f), AmberMat, true);
	}

	for (int32 Index = 0; Index < 3; ++Index)
	{
		const FVector Loc(0.0f, (Index - 1) * 155.0f, 172.0f);
		APointLight* LightActor = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Loc, FRotator::ZeroRotator);
		if (LightActor && LightActor->PointLightComponent)
		{
			LightActor->PointLightComponent->SetLightColor(Index == 1 ? FLinearColor(0.24f, 0.96f, 0.88f) : FLinearColor(1.0f, 0.58f, 0.2f));
			LightActor->PointLightComponent->SetAttenuationRadius(680.0f);
			LightActor->PointLightComponent->SetSourceRadius(36.0f);
			LightActor->PointLightComponent->SetCastShadows(true);
			CoreLights.Add(LightActor);
		}
	}
}

void AKaineCommandLab::BuildWorkstations()
{
	UMaterialInstanceDynamic* DeskMat = MakeMaterial(TEXT("MatteBlackDesk"), FLinearColor(0.018f, 0.021f, 0.023f, 1.0f), 0.65f, 0.34f, 0.0f);
	UMaterialInstanceDynamic* ScreenMat = MakeMaterial(TEXT("KaineScreenGlass"), FLinearColor(0.02f, 0.27f, 0.29f, 1.0f), 0.1f, 0.04f, 1.6f);
	UMaterialInstanceDynamic* KeyMat = MakeMaterial(TEXT("KeyLightAccents"), FLinearColor(0.32f, 0.98f, 0.87f, 1.0f), 0.2f, 0.08f, 1.2f);

	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		AddMesh(FString::Printf(TEXT("OperatorDesk_%d"), Side), CubeMesh, FVector(-250.0f, Side * 360.0f, 50.0f), FRotator(0.0f, Side * 12.0f, 0.0f), FVector(3.2f, 0.82f, 0.26f), DeskMat);
		AddMesh(FString::Printf(TEXT("OperatorMonitor_%d"), Side), CubeMesh, FVector(-290.0f, Side * 336.0f, 122.0f), FRotator(-8.0f, Side * 18.0f, 0.0f), FVector(1.65f, 0.045f, 0.82f), ScreenMat);
		for (int32 Key = 0; Key < 18; ++Key)
		{
			const float X = -390.0f + Key * 11.5f;
			AddMesh(FString::Printf(TEXT("OperatorKey_%d_%02d"), Side, Key), CubeMesh, FVector(X, Side * 292.0f, 78.0f), FRotator::ZeroRotator, FVector(0.06f, 0.018f, 0.012f), KeyMat);
		}
	}
}

void AKaineCommandLab::BuildAuthoredAssetLayer()
{
	UMaterialInstanceDynamic* CarbonMat = MakeMaterial(TEXT("AnodizedCarbonShell"), FLinearColor(0.012f, 0.015f, 0.017f, 1.0f), 0.92f, 0.24f, 0.0f);
	UMaterialInstanceDynamic* GlassBladeMat = MakeMaterial(TEXT("LaserEtchedGlassBlades"), FLinearColor(0.02f, 0.25f, 0.29f, 1.0f), 0.08f, 0.015f, 0.95f);
	UMaterialInstanceDynamic* WarmTrimMat = MakeMaterial(TEXT("MachinedCopperTrim"), FLinearColor(1.0f, 0.52f, 0.18f, 1.0f), 0.82f, 0.18f, 0.35f);
	UMaterialInstanceDynamic* MicroLightMat = MakeMaterial(TEXT("MicroStatusLights"), FLinearColor(0.23f, 0.98f, 0.84f, 1.0f), 0.2f, 0.05f, 2.4f);

	const FVector FaceForward(-0.63f, -0.78f, 0.0f);
	const FVector FaceRight(FaceForward.Y, -FaceForward.X, 0.0f);
	const FVector FaceCenter = FVector(0.0f, 0.0f, 172.0f) + FaceForward * 108.0f;
	const FRotator FaceRot = FaceForward.Rotation();
	const FRotator EyeRot = FaceRight.Rotation();

	AddBeveledPanel(TEXT("KaineFacetedMainVisor"), FaceCenter + FVector(0.0f, 0.0f, 4.0f), FaceRot, 118.0f, 116.0f, 6.0f, 18.0f, GlassBladeMat, true);
	AddBeveledPanel(TEXT("KaineFacetedUpperCrown"), FaceCenter + FVector(0.0f, 0.0f, 76.0f), FaceRot + FRotator(-8.0f, 0.0f, 0.0f), 142.0f, 38.0f, 8.0f, 12.0f, CarbonMat, true);
	AddBeveledPanel(TEXT("KaineFacetedLowerJaw"), FaceCenter + FVector(0.0f, 0.0f, -62.0f), FaceRot + FRotator(9.0f, 0.0f, 0.0f), 102.0f, 40.0f, 8.0f, 12.0f, CarbonMat, true);
	AddBeveledPanel(TEXT("KaineLeftCheekArmor"), FaceCenter - FaceRight * 78.0f + FVector(0.0f, 0.0f, -9.0f), FaceRot + FRotator(0.0f, -12.0f, -7.0f), 42.0f, 108.0f, 7.0f, 9.0f, CarbonMat, true);
	AddBeveledPanel(TEXT("KaineRightCheekArmor"), FaceCenter + FaceRight * 78.0f + FVector(0.0f, 0.0f, -9.0f), FaceRot + FRotator(0.0f, 12.0f, 7.0f), 42.0f, 108.0f, 7.0f, 9.0f, CarbonMat, true);
	AddBeveledPanel(TEXT("KaineLeftEyeGlassInsert"), FaceCenter - FaceRight * 38.0f + FVector(0.0f, 0.0f, 32.0f), EyeRot, 43.0f, 12.0f, 4.0f, 4.0f, MicroLightMat, true);
	AddBeveledPanel(TEXT("KaineRightEyeGlassInsert"), FaceCenter + FaceRight * 38.0f + FVector(0.0f, 0.0f, 32.0f), EyeRot, 43.0f, 12.0f, 4.0f, 4.0f, MicroLightMat, true);
	AddBeveledPanel(TEXT("KaineCopperSpeechGrille"), FaceCenter + FVector(0.0f, 0.0f, -34.0f), EyeRot, 86.0f, 18.0f, 4.0f, 5.0f, WarmTrimMat, true);

	for (int32 Index = 0; Index < 14; ++Index)
	{
		const float Angle = FMath::DegreesToRadians(Index * (360.0f / 14.0f));
		const FVector Radial(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
		const FVector Location = FVector(0.0f, 0.0f, 176.0f) + Radial * 320.0f + FVector(0.0f, 0.0f, FMath::Sin(Angle * 2.0f) * 28.0f);
		const FRotator Rotation = (-Radial).Rotation() + FRotator(0.0f, 0.0f, FMath::Sin(Angle) * 7.0f);
		AddBeveledPanel(FString::Printf(TEXT("KaineSegmentedHaloGlass_%02d"), Index), Location, Rotation, 74.0f, 152.0f, 4.0f, 14.0f, GlassBladeMat, true);
	}

	for (int32 Index = 0; Index < 18; ++Index)
	{
		const float Angle = FMath::DegreesToRadians(Index * 20.0f);
		const FVector Loc(FMath::Cos(Angle) * 188.0f, FMath::Sin(Angle) * 188.0f, 72.0f + (Index % 3) * 12.0f);
		const FRotator Rot(90.0f, FMath::RadiansToDegrees(Angle), 0.0f);
		AddMesh(FString::Printf(TEXT("BraidedCableRun_%02d"), Index), CylinderMesh, Loc, Rot, FVector(0.035f, 0.035f, 2.1f), Index % 2 == 0 ? CarbonMat : WarmTrimMat, true);
	}
}

void AKaineCommandLab::BuildServerWall()
{
	UMaterialInstanceDynamic* RackMat = MakeMaterial(TEXT("RearServerRackCarbon"), FLinearColor(0.014f, 0.017f, 0.02f, 1.0f), 0.78f, 0.26f, 0.0f);
	UMaterialInstanceDynamic* GlassMat = MakeMaterial(TEXT("RearServerSmokedGlass"), FLinearColor(0.02f, 0.16f, 0.18f, 1.0f), 0.15f, 0.05f, 0.6f);
	UMaterialInstanceDynamic* LightMat = MakeMaterial(TEXT("RearServerStatusLights"), FLinearColor(0.2f, 0.95f, 0.82f, 1.0f), 0.22f, 0.06f, 2.0f);
	UMaterialInstanceDynamic* AmberMat = MakeMaterial(TEXT("RearServerAmberStatus"), FLinearColor(1.0f, 0.56f, 0.18f, 1.0f), 0.22f, 0.06f, 1.6f);

	const FRotator RearRot(0.0f, 180.0f, 0.0f);
	for (int32 Col = 0; Col < 5; ++Col)
	{
		for (int32 Row = 0; Row < 4; ++Row)
		{
			const float Y = -420.0f + Col * 210.0f;
			const float Z = 112.0f + Row * 78.0f;
			AddBeveledPanel(FString::Printf(TEXT("RearAuthoredServerRack_%02d_%02d"), Col, Row), FVector(506.0f, Y, Z), RearRot, 162.0f, 52.0f, 8.0f, 10.0f, (Row + Col) % 2 == 0 ? RackMat : GlassMat);
			for (int32 Led = 0; Led < 5; ++Led)
			{
				AddMesh(FString::Printf(TEXT("RearRackLed_%02d_%02d_%02d"), Col, Row, Led), CubeMesh, FVector(497.0f, Y - 58.0f + Led * 15.0f, Z - 12.0f), FRotator::ZeroRotator, FVector(0.018f, 0.018f, 0.018f), Led % 4 == 0 ? AmberMat : LightMat, true);
			}
		}
	}
}

void AKaineCommandLab::BuildHolographicInterface()
{
	UMaterialInstanceDynamic* PanelMat = MakeMaterial(TEXT("PremiumHologramGlass"), FLinearColor(0.015f, 0.18f, 0.20f, 1.0f), 0.08f, 0.02f, 0.9f);
	UMaterialInstanceDynamic* LineMat = MakeMaterial(TEXT("PremiumHologramLines"), FLinearColor(0.22f, 0.96f, 0.88f, 1.0f), 0.12f, 0.03f, 2.2f);
	UMaterialInstanceDynamic* AmberLineMat = MakeMaterial(TEXT("PremiumAmberTelemetry"), FLinearColor(1.0f, 0.58f, 0.18f, 1.0f), 0.16f, 0.05f, 1.7f);
	UMaterialInstanceDynamic* VioletLineMat = MakeMaterial(TEXT("PremiumVioletTelemetry"), FLinearColor(0.62f, 0.46f, 1.0f, 1.0f), 0.16f, 0.05f, 1.5f);
	UMaterialInstanceDynamic* DeepPanelMat = MakeMaterial(TEXT("PremiumDeepPanel"), FLinearColor(0.01f, 0.025f, 0.03f, 1.0f), 0.2f, 0.08f, 0.35f);

	const FRotator LeftRot(0.0f, -26.0f, 0.0f);
	const FRotator RightRot(0.0f, 26.0f, 0.0f);
	const FRotator CenterRot(0.0f, 0.0f, 0.0f);

	HologramPanels.Add(AddMesh(TEXT("LeftGlassCommandPane"), CubeMesh, FVector(-260.0f, -270.0f, 228.0f), LeftRot, FVector(1.95f, 0.018f, 1.18f), PanelMat, true));
	HologramPanels.Add(AddMesh(TEXT("RightGlassCommandPane"), CubeMesh, FVector(-260.0f, 270.0f, 228.0f), RightRot, FVector(1.95f, 0.018f, 1.18f), PanelMat, true));
	HologramPanels.Add(AddMesh(TEXT("CenterFloatingStatusPane"), CubeMesh, FVector(-135.0f, 0.0f, 315.0f), CenterRot, FVector(2.3f, 0.018f, 0.72f), DeepPanelMat, true));

	AddText(TEXT("KaineLabelPrimary"), TEXT("KAINE"), FVector(-228.0f, -291.0f, 292.0f), LeftRot, 22.0f, FLinearColor(0.24f, 0.98f, 0.88f, 1.0f));
	AddText(TEXT("KaineLabelMode"), TEXT("LOCAL OPERATOR CORE"), FVector(-224.0f, -292.0f, 260.0f), LeftRot, 9.5f, FLinearColor(0.78f, 0.92f, 0.92f, 1.0f));
	AddText(TEXT("KaineLabelSecurity"), TEXT("TOOLS LOCKED / PERMISSION LAYER READY"), FVector(-224.0f, -292.0f, 238.0f), LeftRot, 7.8f, FLinearColor(1.0f, 0.64f, 0.24f, 1.0f));
	AddText(TEXT("KaineLabelRightTitle"), TEXT("MISSION STACK"), FVector(-224.0f, 250.0f, 292.0f), RightRot, 13.5f, FLinearColor(1.0f, 0.64f, 0.24f, 1.0f));
	AddText(TEXT("KaineLabelRightBody"), TEXT("3D PRESENCE / MEMORY / VOICE / ACTIONS"), FVector(-224.0f, 250.0f, 264.0f), RightRot, 8.5f, FLinearColor(0.82f, 0.92f, 0.94f, 1.0f));
	AddText(TEXT("KaineLabelCenter"), TEXT("SPEECH ENVELOPE ACTIVE"), FVector(-74.0f, -68.0f, 334.0f), CenterRot, 10.5f, FLinearColor(0.24f, 0.98f, 0.88f, 1.0f));

	for (int32 Panel = 0; Panel < 2; ++Panel)
	{
		const float Side = Panel == 0 ? -1.0f : 1.0f;
		const FRotator Rot = Panel == 0 ? LeftRot : RightRot;
		const FVector Origin(-326.0f, Side * 292.0f, 195.0f);
		for (int32 Row = 0; Row < 9; ++Row)
		{
			const float Z = Origin.Z + Row * 13.0f;
			const float Width = 0.42f + (Row % 3) * 0.18f;
			UMaterialInstanceDynamic* Mat = Row % 4 == 0 ? AmberLineMat : (Row % 5 == 0 ? VioletLineMat : LineMat);
			UStaticMeshComponent* Bar = AddMesh(FString::Printf(TEXT("HoloDataBar_%d_%02d"), Panel, Row), CubeMesh, FVector(Origin.X + Row * 3.0f, Origin.Y, Z), Rot, FVector(Width, 0.012f, 0.014f), Mat, true);
			DataBars.Add(Bar);
		}
	}

	for (int32 Index = 0; Index < 24; ++Index)
	{
		const float Angle = FMath::DegreesToRadians(Index * 15.0f);
		const FVector Loc(-82.0f, FMath::Sin(Angle) * 88.0f, 315.0f + FMath::Cos(Angle) * 18.0f);
		const FRotator Rot(0.0f, FMath::RadiansToDegrees(Angle), 0.0f);
		UStaticMeshComponent* Segment = AddMesh(FString::Printf(TEXT("SpeechWaveSegment_%02d"), Index), CubeMesh, Loc, Rot, FVector(0.11f, 0.009f, 0.04f), Index % 2 == 0 ? LineMat : AmberLineMat, true);
		DataBars.Add(Segment);
	}

	for (int32 Index = 0; Index < 5; ++Index)
	{
		const float Z = 355.0f + Index * 12.0f;
		AddMesh(FString::Printf(TEXT("RearDepthInterfaceLine_%02d"), Index), CubeMesh, FVector(490.0f, 0.0f, Z), FRotator::ZeroRotator, FVector(0.018f, 5.5f - Index * 0.52f, 0.012f), Index % 2 == 0 ? LineMat : VioletLineMat, true);
	}
}

void AKaineCommandLab::BuildPremiumUILayer()
{
	UMaterialInstanceDynamic* CyanMat = MakeMaterial(TEXT("PremiumUICyanInk"), FLinearColor(0.18f, 0.98f, 0.86f, 1.0f), 0.18f, 0.035f, 2.8f);
	UMaterialInstanceDynamic* AmberMat = MakeMaterial(TEXT("PremiumUIAmberInk"), FLinearColor(1.0f, 0.58f, 0.18f, 1.0f), 0.22f, 0.045f, 2.0f);
	UMaterialInstanceDynamic* VioletMat = MakeMaterial(TEXT("PremiumUIVioletInk"), FLinearColor(0.58f, 0.42f, 1.0f, 1.0f), 0.18f, 0.05f, 1.6f);
	UMaterialInstanceDynamic* GlassEdgeMat = MakeMaterial(TEXT("PremiumUISmokedEdge"), FLinearColor(0.015f, 0.045f, 0.052f, 1.0f), 0.48f, 0.065f, 0.42f);
	UMaterialInstanceDynamic* RailMat = MakeMaterial(TEXT("PremiumUIGraphiteRail"), FLinearColor(0.024f, 0.029f, 0.032f, 1.0f), 0.82f, 0.20f, 0.0f);
	UMaterialInstanceDynamic* NodeMat = MakeMaterial(TEXT("PremiumUINodeCore"), FLinearColor(0.72f, 0.95f, 0.92f, 1.0f), 0.16f, 0.025f, 2.4f);
	UMaterialInstanceDynamic* WarningMat = MakeMaterial(TEXT("PremiumUIWarningRed"), FLinearColor(1.0f, 0.22f, 0.16f, 1.0f), 0.18f, 0.055f, 1.7f);

	const FRotator LeftRot(0.0f, -26.0f, 0.0f);
	const FRotator RightRot(0.0f, 26.0f, 0.0f);
	const FRotator CenterRot(0.0f, 0.0f, 0.0f);

	auto AddPanelFrame = [this](const FString& Prefix, const FVector& Center, const FRotator& Rotation, float Width, float Height, UMaterialInterface* Material)
	{
		const FVector Right = Rotation.RotateVector(FVector::XAxisVector);
		const FVector Up = Rotation.RotateVector(FVector::ZAxisVector);
		const FVector Forward = Rotation.RotateVector(FVector::YAxisVector);
		const FVector Offset = Forward * -4.0f;
		const float HalfW = Width * 0.5f;
		const float HalfH = Height * 0.5f;

		PremiumUIElements.Add(AddMesh(Prefix + TEXT("_Top"), CubeMesh, Center + Up * HalfH + Offset, Rotation, FVector(Width / 100.0f, 0.012f, 0.018f), Material, true));
		PremiumUIElements.Add(AddMesh(Prefix + TEXT("_Bottom"), CubeMesh, Center - Up * HalfH + Offset, Rotation, FVector(Width / 100.0f, 0.012f, 0.018f), Material, true));
		PremiumUIElements.Add(AddMesh(Prefix + TEXT("_Left"), CubeMesh, Center - Right * HalfW + Offset, Rotation, FVector(0.018f, 0.012f, Height / 100.0f), Material, true));
		PremiumUIElements.Add(AddMesh(Prefix + TEXT("_Right"), CubeMesh, Center + Right * HalfW + Offset, Rotation, FVector(0.018f, 0.012f, Height / 100.0f), Material, true));
	};

	AddPanelFrame(TEXT("LeftPremiumPaneFrame"), FVector(-260.0f, -270.0f, 228.0f), LeftRot, 236.0f, 152.0f, CyanMat);
	AddPanelFrame(TEXT("RightPremiumPaneFrame"), FVector(-260.0f, 270.0f, 228.0f), RightRot, 236.0f, 152.0f, AmberMat);
	AddPanelFrame(TEXT("CenterPremiumPaneFrame"), FVector(-135.0f, 0.0f, 315.0f), CenterRot, 284.0f, 92.0f, VioletMat);

	PremiumUIElements.Add(AddMesh(TEXT("PremiumTopCommandRailBackplate"), CubeMesh, FVector(-152.0f, -74.0f, 393.0f), CenterRot, FVector(3.35f, 0.018f, 0.28f), RailMat, true));
	const TCHAR* TabLabels[] = { TEXT("CORE"), TEXT("MEMORY"), TEXT("TOOLS"), TEXT("GUARD") };
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const float X = -282.0f + Index * 87.0f;
		UMaterialInstanceDynamic* TabMat = Index == 0 ? CyanMat : (Index == 3 ? AmberMat : GlassEdgeMat);
		PremiumUIElements.Add(AddMesh(FString::Printf(TEXT("PremiumModeTab_%02d"), Index), CubeMesh, FVector(X, -77.0f, 393.0f), CenterRot, FVector(0.68f, 0.012f, 0.18f), TabMat, true));
		AddText(FString::Printf(TEXT("PremiumModeTabLabel_%02d"), Index), TabLabels[Index], FVector(X - 26.0f, -84.0f, 392.0f), CenterRot, 6.2f, Index == 3 ? FLinearColor(1.0f, 0.68f, 0.3f, 1.0f) : FLinearColor(0.82f, 0.98f, 0.95f, 1.0f));
	}

	const FVector ScannerCenter(-88.0f, -172.0f, 250.0f);
	for (int32 Index = 0; Index < 48; ++Index)
	{
		const float Angle = (2.0f * PI * Index) / 48.0f;
		const FVector Radial(FMath::Cos(Angle), 0.0f, FMath::Sin(Angle));
		const FVector Tangent(-FMath::Sin(Angle), 0.0f, FMath::Cos(Angle));
		const float Radius = Index % 2 == 0 ? 82.0f : 67.0f;
		const FVector Location = ScannerCenter + Radial * Radius;
		const FRotator Rotation = Tangent.Rotation();
		UMaterialInstanceDynamic* SegmentMat = Index % 8 == 0 ? AmberMat : (Index % 5 == 0 ? VioletMat : CyanMat);
		PremiumUIElements.Add(AddMesh(FString::Printf(TEXT("PremiumScannerTick_%02d"), Index), CubeMesh, Location, Rotation, FVector(0.12f, 0.009f, 0.012f), SegmentMat, true));
	}
	for (int32 Index = 0; Index < 12; ++Index)
	{
		const float Angle = (2.0f * PI * Index) / 12.0f;
		const FVector End = ScannerCenter + FVector(FMath::Cos(Angle) * 74.0f, 0.0f, FMath::Sin(Angle) * 74.0f);
		PremiumUIElements.Add(AddBeam(FString::Printf(TEXT("PremiumScannerSpoke_%02d"), Index), ScannerCenter, End, 1.0f, Index % 3 == 0 ? AmberMat : CyanMat, true));
	}
	PremiumUIElements.Add(AddMesh(TEXT("PremiumScannerIris"), SphereMesh, ScannerCenter, FRotator::ZeroRotator, FVector(0.12f), NodeMat, true));
	PremiumUIElements.Add(AddMesh(TEXT("PremiumScannerSweepBlade"), CubeMesh, ScannerCenter + FVector(38.0f, -2.0f, 25.0f), FRotator(0.0f, 0.0f, -33.0f), FVector(0.88f, 0.008f, 0.01f), AmberMat, true));
	AddText(TEXT("PremiumScannerLabel"), TEXT("LOCAL MODEL FIELD"), ScannerCenter + FVector(-80.0f, -9.0f, -105.0f), CenterRot, 6.8f, FLinearColor(0.76f, 0.95f, 0.93f, 1.0f));

	const FVector NodePoints[] = {
		FVector(-372.0f, 320.0f, 174.0f),
		FVector(-336.0f, 326.0f, 232.0f),
		FVector(-292.0f, 326.0f, 204.0f),
		FVector(-254.0f, 320.0f, 262.0f),
		FVector(-218.0f, 318.0f, 214.0f),
		FVector(-176.0f, 314.0f, 246.0f),
		FVector(-134.0f, 310.0f, 190.0f)
	};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(NodePoints); ++Index)
	{
		PremiumUIElements.Add(AddMesh(FString::Printf(TEXT("PremiumNeuralNode_%02d"), Index), SphereMesh, NodePoints[Index], FRotator::ZeroRotator, FVector(Index == 3 ? 0.062f : 0.046f), Index % 3 == 0 ? AmberMat : NodeMat, true));
		if (Index > 0)
		{
			PremiumUIElements.Add(AddBeam(FString::Printf(TEXT("PremiumNeuralLink_%02d"), Index), NodePoints[Index - 1], NodePoints[Index], 0.8f, Index % 2 == 0 ? VioletMat : CyanMat, true));
		}
	}
	PremiumUIElements.Add(AddBeam(TEXT("PremiumNeuralCrossLink_A"), NodePoints[1], NodePoints[4], 0.55f, VioletMat, true));
	PremiumUIElements.Add(AddBeam(TEXT("PremiumNeuralCrossLink_B"), NodePoints[2], NodePoints[5], 0.55f, CyanMat, true));
	AddText(TEXT("PremiumNeuralLabel"), TEXT("MEMORY GRAPH"), FVector(-368.0f, 304.0f, 294.0f), RightRot, 7.2f, FLinearColor(1.0f, 0.68f, 0.28f, 1.0f));

	const FVector StatusOrigins[] = {
		FVector(-374.0f, -318.0f, 204.0f),
		FVector(-374.0f, -318.0f, 166.0f),
		FVector(-374.0f, -318.0f, 128.0f)
	};
	const TCHAR* StatusLabels[] = { TEXT("VOICE"), TEXT("TOOLS"), TEXT("SAFE") };
	for (int32 Row = 0; Row < 3; ++Row)
	{
		const FVector Origin = StatusOrigins[Row];
		PremiumUIElements.Add(AddMesh(FString::Printf(TEXT("PremiumStatusCardBack_%02d"), Row), CubeMesh, Origin, LeftRot, FVector(1.16f, 0.012f, 0.22f), GlassEdgeMat, true));
		AddText(FString::Printf(TEXT("PremiumStatusCardLabel_%02d"), Row), StatusLabels[Row], Origin + FVector(-46.0f, -11.0f, 7.0f), LeftRot, 6.1f, Row == 1 ? FLinearColor(1.0f, 0.64f, 0.25f, 1.0f) : FLinearColor(0.78f, 0.96f, 0.93f, 1.0f));
		for (int32 Step = 0; Step < 6; ++Step)
		{
			UMaterialInstanceDynamic* StepMat = (Row == 1 && Step > 2) ? WarningMat : (Step % 4 == 0 ? AmberMat : CyanMat);
			UStaticMeshComponent* Dot = AddMesh(FString::Printf(TEXT("PremiumStatusMicro_%02d_%02d"), Row, Step), CubeMesh, Origin + FVector(-12.0f + Step * 14.0f, -10.0f, -7.0f), LeftRot, FVector(0.075f, 0.009f, 0.018f), StepMat, true);
			PremiumUIElements.Add(Dot);
			DataBars.Add(Dot);
		}
	}

	for (int32 Index = 0; Index < 18; ++Index)
	{
		const float Y = -406.0f + Index * 48.0f;
		const float Z = 66.0f + (Index % 3) * 8.0f;
		PremiumUIElements.Add(AddMesh(FString::Printf(TEXT("PremiumFloorDataTrace_%02d"), Index), CubeMesh, FVector(-194.0f + (Index % 4) * 52.0f, Y, Z), FRotator(0.0f, Index % 2 == 0 ? 0.0f : 90.0f, 0.0f), FVector(0.36f, 0.009f, 0.011f), Index % 5 == 0 ? AmberMat : CyanMat, true));
	}
}

void AKaineCommandLab::BuildLightingAndAtmosphere()
{
	if (UWorld* World = GetWorld())
	{
		ADirectionalLight* Directional = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(-280.0f, -480.0f, 650.0f), FRotator(-34.0f, 44.0f, 0.0f));
		if (Directional && Directional->GetLightComponent())
		{
			Directional->GetLightComponent()->SetIntensity(1.5f);
			Directional->GetLightComponent()->SetLightColor(FLinearColor(0.82f, 0.9f, 1.0f));
		}

		ASkyLight* Sky = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Sky && Sky->GetLightComponent())
		{
			Sky->GetLightComponent()->SetIntensity(0.42f);
			Sky->GetLightComponent()->SetLightColor(FLinearColor(0.58f, 0.68f, 0.8f));
		}

		AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Fog && Fog->GetComponent())
		{
			Fog->GetComponent()->SetFogDensity(0.015f);
			Fog->GetComponent()->SetFogHeightFalloff(0.19f);
			Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(0.08f, 0.16f, 0.18f));
		}

		APostProcessVolume* Post = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Post)
		{
			Post->bUnbound = true;
			Post->Settings.bOverride_BloomIntensity = true;
			Post->Settings.BloomIntensity = 1.25f;
			Post->Settings.bOverride_AutoExposureBias = true;
			Post->Settings.AutoExposureBias = -0.15f;
			Post->Settings.bOverride_VignetteIntensity = true;
			Post->Settings.VignetteIntensity = 0.34f;
			Post->Settings.bOverride_ColorSaturation = true;
			Post->Settings.ColorSaturation = FVector4(0.92f, 0.95f, 0.96f, 1.0f);
			Post->Settings.bOverride_SceneFringeIntensity = true;
			Post->Settings.SceneFringeIntensity = 0.2f;
		}

		for (int32 Index = 0; Index < 6; ++Index)
		{
			const float Y = -500.0f + Index * 200.0f;
			ARectLight* RectActor = World->SpawnActor<ARectLight>(ARectLight::StaticClass(), FVector(120.0f, Y, 456.0f), FRotator(-90.0f, 0.0f, 0.0f));
			if (RectActor && RectActor->RectLightComponent)
			{
				URectLightComponent* Rect = RectActor->RectLightComponent;
				Rect->SetIntensity(460.0f);
				Rect->SetSourceWidth(90.0f);
				Rect->SetSourceHeight(18.0f);
				Rect->SetLightColor(Index % 2 == 0 ? FLinearColor(0.23f, 0.86f, 0.92f) : FLinearColor(1.0f, 0.64f, 0.34f));
				Rect->SetCastShadows(true);
				PanelLights.Add(Rect);
			}
		}
	}
}

void AKaineCommandLab::BuildRing(float Radius, int32 Count, const FVector& AxisA, const FVector& AxisB, const FVector& Center, const FLinearColor& Color)
{
	UMaterialInstanceDynamic* SegmentMat = MakeMaterial(FString::Printf(TEXT("RingMat_%d"), Count), Color, 0.25f, 0.08f, 1.8f);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float Angle = (2.0f * PI * Index) / Count;
		const FVector Radial = AxisA * FMath::Cos(Angle) + AxisB * FMath::Sin(Angle);
		const FVector Tangent = (-AxisA * FMath::Sin(Angle) + AxisB * FMath::Cos(Angle)).GetSafeNormal();
		const FVector Location = Center + Radial * Radius;
		const FRotator Rotation = Tangent.Rotation();
		UStaticMeshComponent* Segment = AddMesh(FString::Printf(TEXT("KaineRing_%03d_%03d"), Count, Index), CubeMesh, Location, Rotation, FVector(0.34f, 0.018f, 0.018f), SegmentMat, true);
		RingSegments.Add(Segment);
	}
}

UStaticMeshComponent* AKaineCommandLab::AddMesh(const FString& Name, UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale, UMaterialInterface* Material, bool bMovable)
{
	UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(this, *Name);
	Component->SetMobility(bMovable ? EComponentMobility::Movable : EComponentMobility::Static);
	Component->SetStaticMesh(Mesh);
	Component->SetRelativeLocation(Location);
	Component->SetRelativeRotation(Rotation);
	Component->SetRelativeScale3D(Scale);
	Component->SetCastShadow(true);
	Component->bReceivesDecals = true;
	if (Material)
	{
		Component->SetMaterial(0, Material);
	}
	Component->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	AddInstanceComponent(Component);
	Component->RegisterComponent();
	return Component;
}

UStaticMeshComponent* AKaineCommandLab::AddBeam(const FString& Name, const FVector& Start, const FVector& End, float Radius, UMaterialInterface* Material, bool bMovable)
{
	const FVector Delta = End - Start;
	const float Length = Delta.Size();
	if (Length <= KINDA_SMALL_NUMBER)
	{
		return nullptr;
	}

	const FVector Midpoint = Start + Delta * 0.5f;
	const FRotator Rotation = FRotationMatrix::MakeFromZ(Delta.GetSafeNormal()).Rotator();
	UStaticMeshComponent* Component = AddMesh(Name, CylinderMesh, Midpoint, Rotation, FVector(Radius / 50.0f, Radius / 50.0f, Length / 100.0f), Material, bMovable);
	if (Component)
	{
		Component->SetCastShadow(false);
	}
	return Component;
}

UProceduralMeshComponent* AKaineCommandLab::AddBeveledPanel(const FString& Name, const FVector& Location, const FRotator& Rotation, float Width, float Height, float Depth, float Chamfer, UMaterialInterface* Material, bool bMovable)
{
	UProceduralMeshComponent* Component = NewObject<UProceduralMeshComponent>(this, *Name);
	Component->SetMobility(bMovable ? EComponentMobility::Movable : EComponentMobility::Static);
	Component->SetRelativeLocation(Location);
	Component->SetRelativeRotation(Rotation);
	Component->bUseAsyncCooking = true;
	Component->SetCastShadow(true);
	if (Material)
	{
		Component->SetMaterial(0, Material);
	}

	const float HalfW = Width * 0.5f;
	const float HalfH = Height * 0.5f;
	const float HalfD = Depth * 0.5f;
	const float C = FMath::Clamp(Chamfer, 1.0f, FMath::Min(HalfW, HalfH) - 1.0f);
	const TArray<FVector2D> Outline = {
		FVector2D(-HalfW + C, -HalfH),
		FVector2D(HalfW - C, -HalfH),
		FVector2D(HalfW, -HalfH + C),
		FVector2D(HalfW, HalfH - C),
		FVector2D(HalfW - C, HalfH),
		FVector2D(-HalfW + C, HalfH),
		FVector2D(-HalfW, HalfH - C),
		FVector2D(-HalfW, -HalfH + C)
	};

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	for (const FVector2D& Point : Outline)
	{
		Vertices.Add(FVector(HalfD, Point.X, Point.Y));
		UV0.Add(FVector2D((Point.X + HalfW) / Width, (Point.Y + HalfH) / Height));
	}
	for (const FVector2D& Point : Outline)
	{
		Vertices.Add(FVector(-HalfD, Point.X, Point.Y));
		UV0.Add(FVector2D((Point.X + HalfW) / Width, (Point.Y + HalfH) / Height));
	}

	for (int32 Index = 1; Index < Outline.Num() - 1; ++Index)
	{
		Triangles.Add(0);
		Triangles.Add(Index);
		Triangles.Add(Index + 1);

		Triangles.Add(Outline.Num());
		Triangles.Add(Outline.Num() + Index + 1);
		Triangles.Add(Outline.Num() + Index);
	}

	for (int32 Index = 0; Index < Outline.Num(); ++Index)
	{
		const int32 Next = (Index + 1) % Outline.Num();
		const int32 FrontA = Index;
		const int32 FrontB = Next;
		const int32 BackA = Outline.Num() + Index;
		const int32 BackB = Outline.Num() + Next;

		Triangles.Add(FrontA);
		Triangles.Add(BackA);
		Triangles.Add(BackB);
		Triangles.Add(FrontA);
		Triangles.Add(BackB);
		Triangles.Add(FrontB);
	}

	Component->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, false, true);
	Component->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	AddInstanceComponent(Component);
	Component->RegisterComponent();
	AuthoredPanels.Add(Component);
	return Component;
}

UTextRenderComponent* AKaineCommandLab::AddText(const FString& Name, const FString& Text, const FVector& Location, const FRotator& Rotation, float WorldSize, const FLinearColor& Color)
{
	UTextRenderComponent* Component = NewObject<UTextRenderComponent>(this, *Name);
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetText(FText::FromString(Text));
	Component->SetTextRenderColor(Color.ToFColor(true));
	Component->SetWorldSize(WorldSize);
	Component->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
	Component->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	Component->SetRelativeLocation(Location);
	Component->SetRelativeRotation(Rotation);
	Component->SetCastShadow(false);
	Component->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	AddInstanceComponent(Component);
	Component->RegisterComponent();
	InterfaceLabels.Add(Component);
	return Component;
}

UMaterialInstanceDynamic* AKaineCommandLab::MakeMaterial(const FString& Name, const FLinearColor& Color, float Metallic, float Roughness, float Emissive)
{
	UMaterialInterface* Source = BaseMaterial ? BaseMaterial.Get() : UMaterial::GetDefaultMaterial(MD_Surface);
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Source, this, *Name);
	Material->SetVectorParameterValue(TEXT("Color"), Color);
	Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
	Material->SetScalarParameterValue(TEXT("Metallic"), Metallic);
	Material->SetScalarParameterValue(TEXT("Roughness"), Roughness);
	Material->SetScalarParameterValue(TEXT("Specular"), 0.8f);
	Material->SetVectorParameterValue(TEXT("Emissive"), Color * Emissive);
	Material->SetScalarParameterValue(TEXT("EmissiveStrength"), Emissive);
	return Material;
}
