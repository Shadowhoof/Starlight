// Shadowhoof Games, 2022


#include "Portal/PortalSurface.h"

#include "Components/CapsuleComponent.h"
#include "Portal/PortalConstants.h"


APortalSurface::APortalSurface()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	RootComponent = StaticMeshComponent;
}

FVector APortalSurface::GetSize() const
{
	return Size;
}

FVector APortalSurface::GetExtents() const
{
	return Size / 2.f;
}

bool APortalSurface::CanFitPortal() const
{
	return bCanFitPortal;
}

TObjectPtr<UPrimitiveComponent> APortalSurface::GetCollisionComponent() const
{
	return StaticMeshComponent;
}

void APortalSurface::BeginPlay()
{
	Super::BeginPlay();

	if (const TObjectPtr<UStaticMesh> StaticMesh = StaticMeshComponent->GetStaticMesh())
	{
		Size = StaticMesh->GetBoundingBox().GetSize() * GetActorScale();
		bCanFitPortal = Size.Y >= PortalConstants::Size.Y && Size.Z >= PortalConstants::Size.Z;
	}
}