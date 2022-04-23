﻿// Shadowhoof Games, 2022


#include "Core/StarlightActor.h"

#include "Portal/Portal.h"

AStarlightActor::AStarlightActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = StaticMeshComponent;
}

TObjectPtr<UPrimitiveComponent> AStarlightActor::GetAttachComponent() const
{
	return StaticMeshComponent;
}

void AStarlightActor::OnOverlapWithPortalBegin(TObjectPtr<APortal> Portal)
{
	ITeleportable::OnOverlapWithPortalBegin(Portal);

	OverlappingPortals.Add(Portal);
	UpdateMaterialParameters();
}

void AStarlightActor::OnOverlapWithPortalEnd(TObjectPtr<APortal> Portal)
{
	ITeleportable::OnOverlapWithPortalEnd(Portal);

	OverlappingPortals.Remove(Portal);
	UpdateMaterialParameters();
}

void AStarlightActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (OverlappingPortals.Num() > 1)
	{
		UpdateMaterialParameters();
	}
}

void AStarlightActor::BeginPlay()
{
	Super::BeginPlay();

	DynamicMaterialInstance = StaticMeshComponent->CreateDynamicMaterialInstance(0);
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

void AStarlightActor::UpdateMaterialParameters()
{
	const int32 PortalCount = OverlappingPortals.Num();
	if (PortalCount == 0)
	{
		DynamicMaterialInstance->SetScalarParameterValue(PortalConstants::InRangeParam, 0.0);
		return;
	}

	// We only consider the closest portal for cull plane because it's such a rare case when an object (and not its copies)
	// is in more than 1 portal at the same time and this case doesn't seem worth wasting time on
	APortal* ClosestPortal;
	if (PortalCount == 1)
	{
		ClosestPortal = OverlappingPortals[0];
	}
	else
	{
		const FVector Location = GetActorLocation();
		const float DistToFirst = FVector::DistSquared(Location, OverlappingPortals[0]->GetActorLocation());
		const float DistToSecond = FVector::DistSquared(Location, OverlappingPortals[1]->GetActorLocation());
		ClosestPortal = DistToFirst < DistToSecond ? OverlappingPortals[0] : OverlappingPortals[1];
	}

	DynamicMaterialInstance->SetScalarParameterValue(PortalConstants::InRangeParam, 1.0);
	DynamicMaterialInstance->SetVectorParameterValue(PortalConstants::LocationParam, ClosestPortal->GetActorLocation());
	DynamicMaterialInstance->SetVectorParameterValue(PortalConstants::NormalParam, ClosestPortal->GetActorForwardVector());
}