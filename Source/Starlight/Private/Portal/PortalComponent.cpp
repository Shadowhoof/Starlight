// Shadowhoof Games, 2022


#include "Portal/PortalComponent.h"

#include "Core/StarlightConstants.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Portal/Portal.h"
#include "Portal/PortalStatics.h"
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
	if (!GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Portal))
	{
		return;
	}

	TObjectPtr<APortalSurface> PortalSurface = Cast<APortalSurface>(HitResult.GetActor());
	if (!PortalSurface)
	{
		return;
	}

	FVector PortalLocation, PortalLocalCoords, PortalExtents;
	FRotator PortalRotation;
	if (!ValidatePortalLocation(PortalType, HitResult, PortalSurface, PortalLocation, PortalLocalCoords, PortalRotation,
	                            PortalExtents))
	{
		UE_LOG(LogPortal, Verbose,
		       TEXT("Could not spawn portal at location %s because it's overlapping with another portal"),
		       *PortalLocation.ToString());
		return;
	}

	// destroy old portal
	const bool bThisPortalExisted = ActivePortals[PortalType] != nullptr;
	if (bThisPortalExisted)
	{
		ActivePortals[PortalType]->Destroy();
	}
	
	EPortalType OtherPortalType = UPortalStatics::GetOtherPortalType(PortalType);
	const TObjectPtr<APortal> OtherPortal = ActivePortals[OtherPortalType];

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	TSubclassOf<APortal> PortalClass = PortalClasses[PortalType];
	const FTransform SpawnTransform = FTransform(PortalRotation, PortalLocation);
	TObjectPtr<APortal> Portal = GetWorld()->SpawnActorDeferred<APortal>(PortalClass, SpawnTransform, nullptr, nullptr,
	                                                                     ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Portal->Initialize(PortalSurface, PortalLocalCoords, PortalExtents, PortalType, OtherPortal);
	UGameplayStatics::FinishSpawningActor(Portal, SpawnTransform);

	ActivePortals[PortalType] = Portal;

	if (OtherPortal)
	{
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

	if (!AreBothPortalsActive())
	{
		return;
	}

	const TObjectPtr<APortal> FirstPortal = ActivePortals[EPortalType::First];
	const TObjectPtr<APortal> SecondPortal = ActivePortals[EPortalType::Second];
	FirstPortal->UpdateSceneCaptureTransform(SecondPortal->GetBackfacingRelativeTransform(OwnerCharacter));
	SecondPortal->UpdateSceneCaptureTransform(FirstPortal->GetBackfacingRelativeTransform(OwnerCharacter));
}

void UPortalComponent::DebugSpawnObjectInPortal(TSubclassOf<AActor> Class)
{
	if (!AreBothPortalsActive())
	{
		return;
	}

	const APortal* Portal = ActivePortals[EPortalType::First];
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector Location = Portal->GetActorLocation() + Portal->GetActorForwardVector() * 5.f - Portal->GetActorUpVector() * 75.f;
	const FRotator Rotation = Portal->GetActorRotation();
	GetWorld()->SpawnActor(Class, &Location, &Rotation, SpawnParams);
}

bool UPortalComponent::AreBothPortalsActive() const
{
	return ActivePortals[EPortalType::First] && ActivePortals[EPortalType::Second];
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
                                              FVector& OutLocalCoords, FRotator& OutRotation, FVector& OutExtents) const
{
	const bool bPortalFits = Surface->
		GetPortalLocation(HitResult, OutLocation, OutLocalCoords, OutExtents, OutRotation);
	if (!bPortalFits)
	{
		return false;
	}

	return !IsOverlappingWithOtherPortal(PortalType, Surface, OutLocalCoords, OutExtents);
}

bool UPortalComponent::IsOverlappingWithOtherPortal(EPortalType PortalType, TObjectPtr<APortalSurface> PortalSurface,
                                                    const FVector& LocalCoords, const FVector& Extents) const
{
	const EPortalType OtherPortalType = UPortalStatics::GetOtherPortalType(PortalType);
	const TObjectPtr<APortal> OtherPortal = ActivePortals[OtherPortalType];
	if (!OtherPortal)
	{
		return false;
	}

	if (OtherPortal->GetPortalSurface() != PortalSurface)
	{
		return false;
	}

	const FVector OtherLocalCoords = OtherPortal->GetLocalCoords();
	const FVector OtherPortalExtents = OtherPortal->GetExtents();
	return FMath::Abs(OtherLocalCoords.Y - LocalCoords.Y) < OtherPortalExtents.Y + Extents.Y &&
		FMath::Abs(OtherLocalCoords.Z - LocalCoords.Z) < OtherPortalExtents.Z + Extents.Z;
}
