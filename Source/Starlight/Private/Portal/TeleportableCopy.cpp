// Shadowhoof Games, 2022


#include "Portal/TeleportableCopy.h"

#include "Portal/Portal.h"
#include "Portal/PortalConstants.h"
#include "Portal/PortalSurface.h"
#include "Portal/CopyStaticMeshComponent.h"
#include "Portal/PortalStatics.h"
#include "Statics/StarlightStatics.h"


ATeleportableCopy::ATeleportableCopy()
{
	StaticMeshComponent = CreateDefaultSubobject<UCopyStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_GrabObstruction, ECR_Ignore);
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->SetNotifyRigidBodyCollision(true);
	RootComponent = StaticMeshComponent;
}

void ATeleportableCopy::Initialize(TObjectPtr<ITeleportable> InParent, TObjectPtr<UStaticMesh> StaticMesh,
                                   TObjectPtr<APortal> OwnerPortal, TObjectPtr<APortal> OtherPortal)
{
	StaticMeshComponent->SetStaticMesh(StaticMesh);
	StaticMeshComponent->SetCollisionObjectType(UPortalStatics::GetCopyObjectType(OwnerPortal->GetPortalType()));
	StaticMeshComponent->SetMassOverrideInKg(NAME_None, InParent->GetCollisionComponent()->GetMass());

	ParentTeleportable = InParent->GetTeleportableScriptInterface();
	ParentActor = InParent->CastToTeleportableActor();

	DynamicMaterialInstance = StaticMeshComponent->CreateDynamicMaterialInstance(0);
	DynamicMaterialInstance->SetScalarParameterValue(PortalConstants::CanBeCulledParam, PortalConstants::FloatTrue);

	UStaticMeshComponent* ParentMesh = Cast<UStaticMeshComponent>(ParentTeleportable->GetCollisionComponent());
	TArray<TObjectPtr<UPrimitiveComponent>> SurfaceCollisionComponents;
	OwnerPortal->GetConnectedPortal()->GetPortalSurface()->GetCollisionComponents(SurfaceCollisionComponents);
	for (UPrimitiveComponent* CollisionComponent : SurfaceCollisionComponents)
	{
		UStarlightStatics::DisableCollisionBetween(StaticMeshComponent, CollisionComponent);
	}

	StaticMeshComponent->SetLinkedComponent(ParentMesh);
}

void ATeleportableCopy::UpdateCullingParams(const FVector& CullPlaneCenter, const FVector& CullPlaneNormal)
{
	DynamicMaterialInstance->SetVectorParameterValue(PortalConstants::CullPlaneCenterParam, CullPlaneCenter);
	DynamicMaterialInstance->SetVectorParameterValue(PortalConstants::CullPlaneNormalParam, CullPlaneNormal);
}

TObjectPtr<AActor> ATeleportableCopy::GetParent() const
{
	return ParentActor;
}

void ATeleportableCopy::ResetVelocity()
{
	StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
}

void ATeleportableCopy::DispatchPhysicsCollisionHit(const FRigidBodyCollisionInfo& MyInfo,
                                                    const FRigidBodyCollisionInfo& OtherInfo,
                                                    const FCollisionImpactData& RigidCollisionData)
{
	Super::DispatchPhysicsCollisionHit(MyInfo, OtherInfo, RigidCollisionData);

	// for now only collide with physics bodies because otherwise we'll be constantly colliding with the floor and
	// propagating that force to the parent
	if (OtherInfo.Component.IsValid() && OtherInfo.Component->GetCollisionObjectType() == ECC_PhysicsBody)
	{
		const FVector TotalImpulse = RigidCollisionData.TotalNormalImpulse;
		const FRigidBodyContactInfo& Contact = RigidCollisionData.ContactInfos[0];
		const FVector Impulse = Contact.ContactNormal.Dot(TotalImpulse.GetSafeNormal()) > 0.f ? TotalImpulse : -TotalImpulse;
		StaticMeshComponent->OnPhysicsImpulseApplied(Impulse, Contact.ContactPosition);
	}
}
