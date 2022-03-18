// Shadowhoof Games, 2022


#include "Portal/PortalComponent.h"

#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Portal/Portal.h"
#include "Portal/PortalSurface.h"


DEFINE_LOG_CATEGORY(LogPortal);


UPortalComponent::UPortalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	PortalClasses = {
		{EPortalType::First, APortal::StaticClass()},
		{EPortalType::Second, APortal::StaticClass()}
	};

	RenderTargets = {
		{EPortalType::First, nullptr},
		{EPortalType::Second, nullptr}
	};
}

void UPortalComponent::ShootPortal(EPortalType PortalType, const FVector& StartLocation, const FVector& Direction)
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

	FRotator PortalRotation = PortalSurface->GetActorRotation();
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
		Portal->SetConnectedPortal(OtherPortal);
		OtherPortal->SetConnectedPortal(Portal);

		Portal->SetRenderTargets(RenderTargets[OtherPortalType], RenderTargets[PortalType]);
		if (!bThisPortalExisted)
		{
			OtherPortal->SetRenderTargets(RenderTargets[PortalType], RenderTargets[OtherPortalType]);
		}
	}
}

void UPortalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ActivePortals[EPortalType::First] || !ActivePortals[EPortalType::Second])
	{
		return;
	}

	const TObjectPtr<APortal> FirstPortal = ActivePortals[EPortalType::First];
	const TObjectPtr<APortal> SecondPortal = ActivePortals[EPortalType::Second];
	FirstPortal->UpdateSceneCaptureTransform(SecondPortal->GetBackfacingRelativeTransform(OwnerCharacter));
	SecondPortal->UpdateSceneCaptureTransform(FirstPortal->GetBackfacingRelativeTransform(OwnerCharacter));
}

void UPortalComponent::BeginPlay()
{
	Super::BeginPlay();

	ActivePortals = {
		{EPortalType::First, nullptr},
		{EPortalType::Second, nullptr}
	};

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (RenderTargets[EPortalType::First] && RenderTargets[EPortalType::Second])
	{
		const TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
		int32 ViewportX, ViewportY;
		PlayerController->GetViewportSize(ViewportX, ViewportY);
		RenderTargets[EPortalType::First]->SizeX = ViewportX;
		RenderTargets[EPortalType::First]->SizeY = ViewportY;
		RenderTargets[EPortalType::Second]->SizeX = ViewportX;
		RenderTargets[EPortalType::Second]->SizeY = ViewportY;
	}
	else
	{
		UE_LOG(LogPortal, Error, TEXT("One or both of the render targets for portals is not set!"));
	}
}

bool UPortalComponent::ValidatePortalLocation(EPortalType PortalType, const FHitResult& HitResult,
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

bool UPortalComponent::IsOverlappingWithOtherPortal(EPortalType PortalType, TObjectPtr<APortalSurface> PortalSurface,
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
