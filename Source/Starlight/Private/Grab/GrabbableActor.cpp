// Shadowhoof Games, 2022


#include "Grab/GrabbableActor.h"


AGrabbableActor::AGrabbableActor()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = StaticMeshComponent;
}

TObjectPtr<UPrimitiveComponent> AGrabbableActor::GetAttachComponent() const
{
	return StaticMeshComponent;
}
