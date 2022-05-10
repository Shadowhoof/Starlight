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

bool APortalSurface::CanFitPortal() const
{
	return bCanFitPortal;
}

bool APortalSurface::GetPortalLocation(const FHitResult& HitResult, FVector& OutLocation, FVector& OutLocalCoords,
                                       FVector& OutExtents, FRotator& OutRotation)
{
	if (!bCanFitPortal)
	{
		return false;
	}

	const FTransform Transform = GetActorTransform();
	if (bFixedOrientation)
	{
		const float YLimit = Extents.Y - PortalConstants::HalfSize.Y;
		const float ZLimit = Extents.Z - PortalConstants::HalfSize.Z;
		OutLocalCoords = Transform.InverseTransformPositionNoScale(HitResult.Location);
		OutLocalCoords.Y = FMath::Clamp(OutLocalCoords.Y, -YLimit, YLimit);
		OutLocalCoords.Z = FMath::Clamp(OutLocalCoords.Z, -ZLimit, ZLimit);

		OutLocation = Transform.TransformPositionNoScale(OutLocalCoords) + HitResult.Normal *
			PortalConstants::OffsetFromSurface;
		OutExtents = PortalConstants::HalfSize;
		OutRotation = GetActorRotation();
	}
	else
	{
		const FVector TraceDir = HitResult.TraceEnd - HitResult.TraceStart;
		const FVector Projection = FVector::VectorPlaneProject(TraceDir, GetActorForwardVector());
		const FVector LocalProjection = Transform.TransformVectorNoScale(Projection.GetSafeNormal());
		float YExtent = FMath::Lerp(PortalConstants::HalfSize.Y, PortalConstants::HalfSize.Z,
		                            FMath::Abs(LocalProjection.Y));
		float ZExtent = FMath::Lerp(PortalConstants::HalfSize.Y, PortalConstants::HalfSize.Z,
		                            FMath::Abs(LocalProjection.Z));

		if (Extents.Y < YExtent || Extents.Z < ZExtent)
		{
			// surface is not big enough to hold portal at this orientation
			return false;
		}

		const float YLimit = Extents.Y - YExtent;
		const float ZLimit = Extents.Z - ZExtent;
		OutLocalCoords = Transform.InverseTransformPositionNoScale(HitResult.Location);
		OutLocalCoords.Y = FMath::Clamp(OutLocalCoords.Y, -YLimit, YLimit);
		OutLocalCoords.Z = FMath::Clamp(OutLocalCoords.Z, -ZLimit, ZLimit);

		OutLocation = Transform.TransformPositionNoScale(OutLocalCoords) + HitResult.Normal *
			PortalConstants::OffsetFromSurface;
		OutExtents = {0.f, YExtent, ZExtent};
		OutRotation = FRotationMatrix::MakeFromXZ(GetActorForwardVector(), Projection).Rotator();
	}

	return true;
}

void APortalSurface::GetCollisionComponents(TArray<TObjectPtr<UPrimitiveComponent>>& OutCollisionComponents)
{
	OutCollisionComponents.Add(StaticMeshComponent);
	if (AttachedSurfaceCollisionComponent)
	{
		OutCollisionComponents.Add(AttachedSurfaceCollisionComponent);
	}
}

void APortalSurface::GetCollisionActors(TArray<TObjectPtr<AActor>>& OutCollisionActors)
{
	OutCollisionActors.Add(this);
	if (AttachedSurface)
	{
		OutCollisionActors.Add(AttachedSurface);
	}
}

TObjectPtr<UPrimitiveComponent> APortalSurface::GetAttachedSurfaceComponent() const
{
	return AttachedSurfaceCollisionComponent;
}

void APortalSurface::BeginPlay()
{
	Super::BeginPlay();

	if (const TObjectPtr<UStaticMesh> StaticMesh = StaticMeshComponent->GetStaticMesh())
	{
		Size = StaticMesh->GetBoundingBox().GetSize() * GetActorScale();
		Extents = Size / 2.f;
		bCanFitPortal = Size.Y >= PortalConstants::Size.Y && Size.Z >= PortalConstants::Size.Z;
	}
	else
	{
		UE_LOG(LogPortal, Error, TEXT("Portal surface %s has no static mesh"), *GetName());
	}

	if (AttachedSurface)
	{
		AttachedSurfaceCollisionComponent = Cast<
			UPrimitiveComponent>(AttachedSurface->GetComponentByClass(UPrimitiveComponent::StaticClass()));
	}
}
