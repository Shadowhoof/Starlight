// Shadowhoof Games, 2022


#include "Portal/PortalSurface.h"

#include "Components/CapsuleComponent.h"
#include "Portal/PortalConstants.h"
#include "Statics/StarlightMacros.h"


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

void APortalSurface::SetCollisionEnabledForActor(const TObjectPtr<AActor> Actor, bool bIsEnabled)
{
	UE_LOG(LogPortal, Verbose, TEXT("Setting collision for portal surface %s and actor %s, enabled: %s"), *GetName(),
	       *Actor->GetName(), BOOL_TO_STRING(bIsEnabled));

	const bool bIgnoreCollision = !bIsEnabled;
	StaticMeshComponent->IgnoreActorWhenMoving(Actor, bIgnoreCollision);

	TInlineComponentArray<UPrimitiveComponent*> OtherActorComponents;
	Actor->GetComponents<UPrimitiveComponent>(OtherActorComponents);
	for (UPrimitiveComponent* Component : OtherActorComponents)
	{
		Component->IgnoreActorWhenMoving(this, bIgnoreCollision);
	}
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
