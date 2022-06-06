// Shadowhoof Games, 2022


#include "Portal/TeleportableCopy.h"

#include "Portal/Portal.h"
#include "Portal/PortalConstants.h"
#include "Portal/PortalSurface.h"
#include "Statics/StarlightStatics.h"


ATeleportableCopy::ATeleportableCopy()
{
}

void ATeleportableCopy::Initialize(TObjectPtr<ITeleportable> InParent, TObjectPtr<APortal> InOwnerPortal)
{
	OwnerPortal = InOwnerPortal;
	ParentActor = InParent->CastToTeleportableActor();
}

void ATeleportableCopy::UpdateCullingParams(const FVector& CullPlaneCenter, const FVector& CullPlaneNormal)
{
	for (UMaterialInstanceDynamic* Instance : DynamicMaterialInstances)
	{
		Instance->SetVectorParameterValue(PortalConstants::CullPlaneCenterParam, CullPlaneCenter);
		Instance->SetVectorParameterValue(PortalConstants::CullPlaneNormalParam, CullPlaneNormal);
	}
}

TObjectPtr<AActor> ATeleportableCopy::GetParent() const
{
	return ParentActor;
}

void ATeleportableCopy::ResetVelocity()
{
}

TWeakObjectPtr<APortal> ATeleportableCopy::GetOwnerPortal() const
{
	return OwnerPortal;
}

void ATeleportableCopy::DisableCollisionWithPortal(TObjectPtr<UPrimitiveComponent> CollisionComponent)
{
	TArray<TObjectPtr<UPrimitiveComponent>> SurfaceCollisionComponents;
	OwnerPortal->GetConnectedPortal()->GetPortalSurface()->GetCollisionComponents(SurfaceCollisionComponents);
	for (UPrimitiveComponent* SurfaceCollisionComponent : SurfaceCollisionComponents)
	{
		UStarlightStatics::DisableCollisionBetween(SurfaceCollisionComponent, CollisionComponent);
	}
}

void ATeleportableCopy::CreateDynamicInstances(TObjectPtr<UMeshComponent> MeshComponent)
{
	const int32 MaterialCount = MeshComponent->GetMaterials().Num();
	DynamicMaterialInstances.Reset(MaterialCount);
	for (int32 Index = 0; Index < MaterialCount; ++Index)
	{
		UMaterialInstanceDynamic* Instance = MeshComponent->CreateAndSetMaterialInstanceDynamic(Index);
		Instance->SetScalarParameterValue(PortalConstants::CanBeCulledParam, PortalConstants::FloatTrue);
		DynamicMaterialInstances.Add(Instance);
	}
}
