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
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StaticMesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	FVector Size;

	bool bCanFitPortal = false;
	
protected:

	virtual void BeginPlay() override;
	
	
};
