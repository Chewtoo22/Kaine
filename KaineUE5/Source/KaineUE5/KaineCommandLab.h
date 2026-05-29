#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KaineCommandLab.generated.h"

class APointLight;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UProceduralMeshComponent;
class URectLightComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class KAINEUE5_API AKaineCommandLab : public AActor
{
	GENERATED_BODY()

public:
	AKaineCommandLab();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void BuildRoom();
	void BuildKaineCore();
	void BuildWorkstations();
	void BuildLightingAndAtmosphere();
	void BuildHolographicInterface();
	void BuildAuthoredAssetLayer();
	void BuildPremiumUILayer();
	void BuildServerWall();
	void BuildRing(float Radius, int32 Count, const FVector& AxisA, const FVector& AxisB, const FVector& Center, const FLinearColor& Color);

	UStaticMeshComponent* AddMesh(const FString& Name, UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale, UMaterialInterface* Material, bool bMovable = false);
	UStaticMeshComponent* AddBeam(const FString& Name, const FVector& Start, const FVector& End, float Radius, UMaterialInterface* Material, bool bMovable = true);
	UProceduralMeshComponent* AddBeveledPanel(const FString& Name, const FVector& Location, const FRotator& Rotation, float Width, float Height, float Depth, float Chamfer, UMaterialInterface* Material, bool bMovable = false);
	UTextRenderComponent* AddText(const FString& Name, const FString& Text, const FVector& Location, const FRotator& Rotation, float WorldSize, const FLinearColor& Color);
	UMaterialInstanceDynamic* MakeMaterial(const FString& Name, const FLinearColor& Color, float Metallic, float Roughness, float Emissive);

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SphereMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> PlaneMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BaseMaterial;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CoreSphere;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> RingSegments;

	UPROPERTY()
	TArray<TObjectPtr<APointLight>> CoreLights;

	UPROPERTY()
	TArray<TObjectPtr<URectLightComponent>> PanelLights;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> HologramPanels;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> DataBars;

	UPROPERTY()
	TArray<TObjectPtr<UTextRenderComponent>> InterfaceLabels;

	UPROPERTY()
	TArray<TObjectPtr<UProceduralMeshComponent>> AuthoredPanels;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PremiumUIElements;
};
