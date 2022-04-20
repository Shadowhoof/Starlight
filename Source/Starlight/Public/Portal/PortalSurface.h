// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalSurface.generated.h"

UCLASS()
class STARLIGHT_API APortalSurface : public AActor
{
	GENERATED_BODY()

public:
	
	APortalSurface();

	bool CanFitPortal() const;

	bool GetPortalLocation(const FHitResult& HitResult, FVector& OutLocation, FVector& OutLocalCoords, FVector& OutExtents, FRotator& OutRotation);
	
	/**
	 * Returns components that teleportable actor has to disable collision with in order to pass through the portal.
	 * Can include portal's own components as well as components of the surface it is attached to.
	 */
	void GetCollisionComponents(TArray<TObjectPtr<UPrimitiveComponent>>& OutCollisionComponents);

	TObjectPtr<UPrimitiveComponent> GetAttachedSurfaceComponent() const;
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StaticMesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<AActor> AttachedSurface;

	/**
	 * Indicates whether portal is always created at a certain orientation which is the same as surface orientation.
	 * If <code>false</code> then portal orientation is determined by player orientation when portal is created.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	bool bFixedOrientation = true;
	
	FVector Size;
	FVector Extents;

	bool bCanFitPortal = false;
	
protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> AttachedSurfaceCollisionComponent;
	
};
