﻿// Shadowhoof Games, 2022


#include "Portal/PortalGunComponent.h"

#include "Portal/Portal.h"
#include "Portal/PortalSurface.h"


DEFINE_LOG_CATEGORY(LogPortal);


UPortalGunComponent::UPortalGunComponent()
{
	PortalClasses = {
		{EPortalType::First, APortal::StaticClass()},
		{EPortalType::Second, APortal::StaticClass()}
	};

	RenderTargets = {
		{EPortalType::First, nullptr},
		{EPortalType::Second, nullptr}
	};
}

void UPortalGunComponent::ShootPortal(EPortalType PortalType, const FVector& StartLocation, const FVector& Direction)
{
	FHitResult HitResult;
	const FVector EndLocation = StartLocation + Direction * PortalConstants::ShootRange;
	GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Portal);
	if (!HitResult.IsValidBlockingHit())
	{
		return;
	}

	TObjectPtr<APortalSurface> PortalSurface = Cast<APortalSurface>(HitResult.GetActor());
	if (!PortalSurface || !PortalSurface->CanFitPortal())
	{
		return;
	}

	FVector PortalLocation, LocalCoords;
	if (!ValidatePortalLocation(PortalType, HitResult, PortalSurface, PortalLocation, LocalCoords))
	{
		UE_LOG(LogPortal, Verbose,
		       TEXT("Could not spawn portal at location %s because it's overlapping with another portal"),
		       *PortalLocation.ToString());
		return;
	}

	FRotator PortalRotation = HitResult.Normal.Rotation();
	PortalRotation.Pitch -= 90.f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	TSubclassOf<APortal> PortalClass = PortalClasses[PortalType];
	TObjectPtr<APortal> Portal = GetWorld()->SpawnActor<APortal>(
		PortalClass,
		PortalLocation,
		PortalRotation,
		SpawnParams
	);
	Portal->Initialize(PortalSurface, LocalCoords);

	const bool bThisPortalExisted = ActivePortals[PortalType] != nullptr;
	if (bThisPortalExisted)
	{
		ActivePortals[PortalType]->Destroy();
	}

	ActivePortals[PortalType] = Portal;

	EPortalType OtherPortalType = GetOtherPortalType(PortalType);
	if (const TObjectPtr<APortal> OtherPortal = ActivePortals[OtherPortalType])
	{
		Portal->SetRenderTargets(RenderTargets[OtherPortalType], RenderTargets[PortalType]);
		if (!bThisPortalExisted)
		{
			OtherPortal->SetRenderTargets(RenderTargets[PortalType], RenderTargets[OtherPortalType]);
		}
	}
}

void UPortalGunComponent::BeginPlay()
{
	Super::BeginPlay();

	ActivePortals = {
		{EPortalType::First, nullptr},
		{EPortalType::Second, nullptr}
	};
}

bool UPortalGunComponent::ValidatePortalLocation(EPortalType PortalType, const FHitResult& HitResult,
                                                 TObjectPtr<APortalSurface> Surface, FVector& OutLocation,
                                                 FVector& OutLocalCoords) const
{
	const FTransform SurfaceTransform = Surface->GetActorTransform();
	const FVector SurfaceExtents = Surface->GetExtents();

	const float YLimit = SurfaceExtents.Y - PortalConstants::HalfSize.Y;
	const float ZLimit = SurfaceExtents.Z - PortalConstants::HalfSize.Z;
	OutLocalCoords = SurfaceTransform.InverseTransformPositionNoScale(HitResult.Location);
	OutLocalCoords.Y = FMath::Clamp(OutLocalCoords.Y, -YLimit, YLimit);
	OutLocalCoords.Z = FMath::Clamp(OutLocalCoords.Z, -ZLimit, ZLimit);

	OutLocation = SurfaceTransform.TransformPositionNoScale(OutLocalCoords) + HitResult.Normal *
		PortalConstants::OffsetFromSurface;
	return !IsOverlappingWithOtherPortal(PortalType, Surface, OutLocalCoords);
}

bool UPortalGunComponent::IsOverlappingWithOtherPortal(EPortalType PortalType, TObjectPtr<APortalSurface> PortalSurface,
                                                       const FVector& LocalCoords) const
{
	const TObjectPtr<APortal> OtherPortal = ActivePortals[GetOtherPortalType(PortalType)];
	if (!OtherPortal)
	{
		return false;
	}

	if (OtherPortal->GetPortalSurface() != PortalSurface)
	{
		return false;
	}

	const FVector OtherLocalCoords = OtherPortal->GetLocalCoords();
	return FMath::Abs(OtherLocalCoords.Y - LocalCoords.Y) < PortalConstants::Size.Y &&
		FMath::Abs(OtherLocalCoords.Z - LocalCoords.Z) < PortalConstants::Size.Z;
}
