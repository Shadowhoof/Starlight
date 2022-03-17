// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"


class UBoxComponent;
class APortalSurface;

UCLASS()
class STARLIGHT_API APortal : public AActor
{
	GENERATED_BODY()

public:
	APortal();

	virtual void Tick(float DeltaSeconds) override;
	
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
	void SetRenderTargets(TObjectPtr<UTextureRenderTarget2D> ReadTarget,
	                      TObjectPtr<UTextureRenderTarget2D> WriteTarget);

	void UpdateSceneCaptureTransform(const FVector& RelativeLocation);

	FVector GetRelativeLocationTo(TObjectPtr<ACharacter> PlayerCharacter) const;

	void SetConnectedPortal(TObjectPtr<APortal> Portal);

	void PrepareForActorTeleport(TObjectPtr<AActor> TeleportingActor);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UStaticMeshComponent> PortalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UStaticMeshComponent> BorderMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UBoxComponent> CollisionBoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<USceneComponent> BackFacingComponent;

protected:
	virtual void BeginPlay() override;

private:
	
	UPROPERTY()
	TObjectPtr<APortalSurface> PortalSurface = nullptr;

	UPROPERTY()
	TObjectPtr<APortal> OtherPortal = nullptr;
	
	UPROPERTY()
	FVector LocalCoords;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicInstance = nullptr;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTargetWrite = nullptr;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTargetRead = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActorsInPortalRange;
	
private:
	UFUNCTION()
	void OnCollisionBoxStartOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                           const FHitResult& SweepResult);

	UFUNCTION()
	void OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void OnActorBeginOverlap(TObjectPtr<AActor> Actor);
	void OnActorEndOverlap(TObjectPtr<AActor> Actor);
	
	void TeleportActor(TObjectPtr<AActor> Actor);
};
