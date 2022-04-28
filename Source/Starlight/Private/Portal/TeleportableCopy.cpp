// Shadowhoof Games, 2022


#include "Portal/TeleportableCopy.h"

#include "Portal/PortalConstants.h"


ATeleportableCopy::ATeleportableCopy()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetSimulatePhysics(false);
	RootComponent = StaticMeshComponent;
}

void ATeleportableCopy::Initialize(TObjectPtr<AActor> InParent, TObjectPtr<UStaticMesh> StaticMesh, EPortalType PortalType)
{
	StaticMeshComponent->SetStaticMesh(StaticMesh);
	StaticMeshComponent->SetCollisionObjectType(GetCopyObjectType(PortalType));

	Parent = InParent;

	DynamicMaterialInstance = StaticMeshComponent->CreateDynamicMaterialInstance(0);
	DynamicMaterialInstance->SetScalarParameterValue(PortalConstants::CanBeCulledParam, PortalConstants::FloatTrue);
}

void ATeleportableCopy::UpdateCullingParams(const FVector& CullPlaneCenter, const FVector& CullPlaneNormal)
{
	DynamicMaterialInstance->SetVectorParameterValue(PortalConstants::CullPlaneCenterParam, CullPlaneCenter);
	DynamicMaterialInstance->SetVectorParameterValue(PortalConstants::CullPlaneNormalParam, CullPlaneNormal);
}

TObjectPtr<AActor> ATeleportableCopy::GetParent() const
{
	return Parent;
}

