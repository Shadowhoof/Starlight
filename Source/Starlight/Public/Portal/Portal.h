// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"


class APortalSurface;

UCLASS()
class STARLIGHT_API APortal : public AActor
{
	GENERATED_BODY()

public:
	APortal();

	/**
	 *	Fills out portal data. Must be called immediately after creating a new portal.
	 *	@param Surface actor the portal is attached to
	 *	@param InLocalCoords local coordinates of portal within portal surface
	 */
	void Initialize(const TObjectPtr<APortalSurface> Surface, FVector InLocalCoords);

	/** Returns surface the portal is attached to. */
	TObjectPtr<APortalSurface> GetPortalSurface() const;

	/** Returns portal local coordinates within portal surface. */
	FVector GetLocalCoords() const;

	/**
	 *	Sets render targets for portal
	 *	@param ReadTarget render target that will be used as portal material
	 *	@param WriteTarget render target that will be used as target for scene capture component
	 */
	void SetRenderTargets(TObjectPtr<UTextureRenderTarget2D> ReadTarget, TObjectPtr<UTextureRenderTarget2D> WriteTarget);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UStaticMeshComponent> PortalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UStaticMeshComponent> BorderMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UTexture2D> NoConnectedPortalTexture;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<APortalSurface> PortalSurface = nullptr;

	UPROPERTY()
	FVector LocalCoords;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicInstance = nullptr;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTargetWrite = nullptr;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTargetRead = nullptr;
};
