// Shadowhoof Games, 2022


#include "Portal/CopyStaticMeshComponent.h"


UCopyStaticMeshComponent::UCopyStaticMeshComponent()
{
}

void UCopyStaticMeshComponent::SetLinkedComponent(TObjectPtr<UPrimitiveComponent> Component)
{
	ensureMsgf(!LinkedComponent, TEXT("Linked component is already assigned to static mesh of %s"), *GetOwner()->GetName());
	LinkedComponent = Component;
}

void UCopyStaticMeshComponent::AddForceAtLocation(FVector Force, FVector Location, FName BoneName)
{
	Super::AddForceAtLocation(Force, Location, BoneName);

	if (LinkedComponent)
	{
		const FVector LocalForce = GetComponentTransform().InverseTransformVectorNoScale(Force);
		const FVector TransformedForce = LinkedComponent->GetComponentTransform().TransformVectorNoScale(LocalForce);
		const FVector LocalLocation = GetComponentTransform().InverseTransformPositionNoScale(Location);
		const FVector TransformedLocation = LinkedComponent->GetComponentTransform().TransformPositionNoScale(LocalLocation);
		LinkedComponent->AddForceAtLocation(TransformedForce, TransformedLocation, BoneName);
	}
}

void UCopyStaticMeshComponent::AddImpulseAtLocation(FVector Impulse, FVector Location, FName BoneName)
{
	Super::AddImpulseAtLocation(Impulse, Location, BoneName);

	PropagateImpulseAtLocation(Impulse, Location, BoneName);
}

void UCopyStaticMeshComponent::OnPhysicsImpulseApplied(const FVector& Impulse, const FVector& Location)
{
	PropagateImpulseAtLocation(Impulse, Location);
}

void UCopyStaticMeshComponent::PropagateImpulseAtLocation(const FVector& Impulse, const FVector& Location, FName BoneName)
{
	if (LinkedComponent)
	{
		const FVector LocalImpulse = GetComponentTransform().InverseTransformVectorNoScale(Impulse);
		const FVector TransformedImpulse = LinkedComponent->GetComponentTransform().TransformVectorNoScale(LocalImpulse);
		const FVector LocalLocation = GetComponentTransform().InverseTransformPositionNoScale(Location);
		const FVector TransformedLocation = LinkedComponent->GetComponentTransform().TransformPositionNoScale(LocalLocation);
		LinkedComponent->AddImpulseAtLocation(TransformedImpulse, TransformedLocation, BoneName);
	}
}
