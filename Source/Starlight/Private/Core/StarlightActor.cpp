// Shadowhoof Games, 2022


#include "Core/StarlightActor.h"


AStarlightActor::AStarlightActor()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = StaticMeshComponent;
}

TObjectPtr<UPrimitiveComponent> AStarlightActor::GetAttachComponent() const
{
	return StaticMeshComponent;
}

FRotator AStarlightActor::GetTeleportRotation()
{
	return GetActorRotation();
}

bool AStarlightActor::SetTeleportLocationAndRotation(const FVector& Location, const FRotator& Rotation)
{
	return TeleportTo(Location, Rotation, false, true);
}

TObjectPtr<UPrimitiveComponent> AStarlightActor::GetCollisionComponent() const
{
	return StaticMeshComponent;
}

FVector AStarlightActor::GetVelocity() const
{
	return StaticMeshComponent->GetPhysicsLinearVelocity();
}

void AStarlightActor::SetVelocity(const FVector& Velocity)
{
	StaticMeshComponent->SetPhysicsLinearVelocity(Velocity);
}
