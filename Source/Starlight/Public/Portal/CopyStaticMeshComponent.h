// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "CopyStaticMeshComponent.generated.h"

class AStarlightActor;
class ATeleportableCopy;

/**
 * Static mesh component which is used by a copy of StarlightActor which was created by a portal.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STARLIGHT_API UCopyStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UCopyStaticMeshComponent();

	/**
	 * Sets a linked component to which this component will propagate all forces applied to it.
	 */
	void SetLinkedComponent(TObjectPtr<UPrimitiveComponent> Component);
	
	virtual void AddForceAtLocation(FVector Force, FVector Location, FName BoneName) override;
	virtual void AddImpulseAtLocation(FVector Impulse, FVector Location, FName BoneName) override;
	void OnPhysicsImpulseApplied(const FVector& Impulse, const FVector& Location);
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UPrimitiveComponent> LinkedComponent;

private:

	void PropagateImpulseAtLocation(const FVector& Impulse, const FVector& Location, FName BoneName = NAME_None);
	
};
