// Shadowhoof Games, 2022


#include "Portal/StaticTeleportableCopy.h"

#include "Portal/CopyStaticMeshComponent.h"
#include "Portal/Portal.h"
#include "Portal/PortalStatics.h"


AStaticTeleportableCopy::AStaticTeleportableCopy()
{
	StaticMeshComponent = CreateDefaultSubobject<UCopyStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->SetNotifyRigidBodyCollision(true);
	RootComponent = StaticMeshComponent;
}

void AStaticTeleportableCopy::Initialize(TObjectPtr<ITeleportable> InParent, TObjectPtr<APortal> InOwnerPortal)
{
	Super::Initialize(InParent, InOwnerPortal);
	
	UStaticMeshComponent* ParentMeshComponent = Cast<UStaticMeshComponent>(ParentActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
	StaticMeshComponent->SetStaticMesh(ParentMeshComponent->GetStaticMesh());

	UPrimitiveComponent* ParentCollisionComponent = InParent->GetCollisionComponent();
	StaticMeshComponent->SetCollisionObjectType(UPortalStatics::GetCopyObjectType(InOwnerPortal->GetPortalType()));
	StaticMeshComponent->SetMassOverrideInKg(NAME_None, ParentCollisionComponent->GetBodyInstance()->GetBodyMass());
	StaticMeshComponent->SetLinkedComponent(ParentCollisionComponent);

	StaticMeshComponent->SetCollisionResponseToChannels(ParentCollisionComponent->GetCollisionResponseToChannels());
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_GrabObstruction, ECR_Ignore);

	DisableCollisionWithPortal(StaticMeshComponent);
	CreateDynamicInstances(StaticMeshComponent);
}

void AStaticTeleportableCopy::ResetVelocity()
{
	StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
}

void AStaticTeleportableCopy::DispatchPhysicsCollisionHit(const FRigidBodyCollisionInfo& MyInfo,
													const FRigidBodyCollisionInfo& OtherInfo,
													const FCollisionImpactData& RigidCollisionData)
{
	Super::DispatchPhysicsCollisionHit(MyInfo, OtherInfo, RigidCollisionData);

	// for now only collide with certain object types because otherwise we'll be constantly colliding with the floor and
	// propagating that force to the parent
	static const TSet PropagatedObjectTypes = {ECC_PhysicsBody, ECC_Pawn, ECC_WithinFirstPortal, ECC_WithinSecondPortal, ECC_WithinBothPortals};
	if (OtherInfo.Component.IsValid() && PropagatedObjectTypes.Contains(OtherInfo.Component->GetCollisionObjectType()))
	{
		const FVector TotalImpulse = RigidCollisionData.TotalNormalImpulse;
		const FRigidBodyContactInfo& Contact = RigidCollisionData.ContactInfos[0];
		const FVector Impulse = Contact.ContactNormal.Dot(TotalImpulse.GetSafeNormal()) > 0.f ? TotalImpulse : -TotalImpulse;
		StaticMeshComponent->OnPhysicsImpulseApplied(Impulse, Contact.ContactPosition);
	}
}
