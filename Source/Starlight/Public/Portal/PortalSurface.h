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

	/** Returns total surface size from one corner to the opposite. */
	FVector GetSize() const;

	/** Returns a vector containing distance from center to corner. */
	FVector GetExtents() const;

	bool CanFitPortal() const;

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

	FVector Size;

	bool bCanFitPortal = false;
	
protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> AttachedSurfaceCollisionComponent;
	
};
