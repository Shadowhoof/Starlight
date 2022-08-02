// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalSurface.generated.h"

class UArrowComponent;

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

	/**
	 * Returns actors that teleportable actor has to disable collision with in order to pass through the portal.
	 */
	void GetCollisionActors(TArray<TObjectPtr<AActor>>& OutCollisionActors);
	
	TObjectPtr<UPrimitiveComponent> GetAttachedSurfaceComponent() const;
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StaticMesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> ArrowComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<AActor> AttachedSurface;

	/**
	 * Indicates whether portal is always created at a certain orientation which is the same as surface orientation.
	 * If <code>false</code> then portal orientation is determined by player orientation when portal is created.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	bool bFixedOrientation = true;

	/** Indicates whether portal can only be placed in the center of the surface. Hitting any point of the surface with
	 * the portal gun will try to place the portal in the center. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	bool bPortalOnlyInCenter = false;
	
	FVector Size;
	FVector Extents;

	bool bCanFitPortal = false;
	
protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> AttachedSurfaceCollisionComponent;
	
};
