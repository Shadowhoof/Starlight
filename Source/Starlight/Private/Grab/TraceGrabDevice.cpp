// Shadowhoof Games, 2022


#include "Grab/TraceGrabDevice.h"

#include "Core/StarlightCharacter.h"
#include "Camera/CameraComponent.h"
#include "Core/StarlightActor.h"
#include "Core/StarlightGameMode.h"
#include "Grab/Grabbable.h"
#include "Portal/Portal.h"
#include "Portal/PortalStatics.h"
#include "Portal/TeleportableCopy.h"

namespace TraceGrabConstants
{
	const float GrabRange = 250.f;

	const FVector HeldObjectOffset = {150.f, 0.f, 0.f};
	const float MaxHoldDistance = 250.f;
	const float MinHoldDotProduct = FMath::Cos(FMath::DegreesToRadians(60.f));
	const float NoSightReleaseDelay = 0.5f;
	
	const float MaxMovementSpeed = 1000.f;
	const float MaxRotationSpeedInRadians = FMath::DegreesToRadians(1080.f);
}


bool UTraceGrabDevice::TryGrabbing()
{
	if (GrabbedObject)
	{
		return false;
	}
	
	FHitResult HitResult;
	const FVector StartPoint = OwnerComponent->GetComponentLocation();
	const FVector EndPoint = StartPoint + OwnerComponent->GetComponentRotation().Vector() *
		TraceGrabConstants::GrabRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerCharacter);
	TArray<TObjectPtr<APortal>> CrossedPortals;
	bool bBlockingHit = UPortalStatics::LineTraceThroughPortal(GetWorld(), HitResult, StartPoint, EndPoint,
	                                                           ECC_PhysicsBody, &CrossedPortals, QueryParams);
	if (!bBlockingHit)
	{
		return false;
	}

	AActor* HitActor = HitResult.GetActor();
	IGrabbable* Grabbable = Cast<IGrabbable>(HitActor);
	if (!Grabbable)
	{
		// Maybe we've hit a teleportable copy. If yes, try to grab its parent.
		ATeleportableCopy* TeleportableCopy = Cast<ATeleportableCopy>(HitActor);
		if (TeleportableCopy)
		{
			Grabbable = Cast<IGrabbable>(TeleportableCopy->GetParent());
			APortal* Portal = TeleportableCopy->GetOwnerPortal().Get();
			if (Portal)
			{
				CrossedPortals.Add(Portal->GetConnectedPortal());
			}
		}
	}

	if (!Grabbable)
	{
		UE_LOG(LogGrab, Verbose, TEXT("Grab trace has hit non-grabbable actor %s"), *HitActor->GetName());
		return false;
	}

	UE_LOG(LogGrab, Verbose, TEXT("Grab trace has hit grabbable actor %s"), *HitResult.GetActor()->GetName());
	if (!Grab(Grabbable))
	{
		return false;
	}

	HeldThroughPortals.Append(CrossedPortals);
	return true;
}

void UTraceGrabDevice::Initialize(TObjectPtr<USceneComponent> InOwnerComponent)
{
	Super::Initialize(InOwnerComponent);
	PlayerCharacter = Cast<AStarlightCharacter>(InOwnerComponent->GetOwner());

	AStarlightGameMode* GameMode = GetWorld()->GetAuthGameMode<AStarlightGameMode>();
	if (GameMode)
	{
		GameMode->OnActorTeleported().AddUObject(this, &UTraceGrabDevice::OnActorTeleported);
	}
}

void UTraceGrabDevice::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!GrabbedObject)
	{
		return;
	}

	/* TODO: Code in this method is not very well optimized. There are multiple "teleportations" and "unteleportations"
	 * through portals which could potentially be done just once and then we'd use the results wherever we need them. */
	
	// check line of sight to owner component
	if (!ShouldKeepHoldingObject())
	{
		if (!bIsPendingRelease)
		{
			bIsPendingRelease = true;
			ReleaseDelay = TraceGrabConstants::NoSightReleaseDelay;
		}
		else
		{
			ReleaseDelay -= DeltaSeconds;
		}

		if (ReleaseDelay <= 0.f)
		{
			Release();
			return;
		}
	}
	else if (bIsPendingRelease)
	{
		bIsPendingRelease = false;
	}

	// calculate new desired location for grabbed object
	UPrimitiveComponent* GrabbedComponent = GrabbedObject->GetComponentToGrab();
	const FVector GrabbedComponentLocation = GrabbedComponent->GetComponentLocation(); 
	const FVector ToDesiredLocation = GetDesiredGrabbedObjectLocation() - GrabbedComponentLocation;
	const float MaxMovementDelta = TraceGrabConstants::MaxMovementSpeed * DeltaSeconds;
	const FVector MovementDelta = ToDesiredLocation.GetClampedToMaxSize(MaxMovementDelta);

	// calculate new desired rotation for grabbed object
	const FQuat CurrentRotation = GrabbedComponent->GetComponentQuat();
	const FQuat DesiredRotation = GetDesiredGrabbedObjectRotation();
	const float RotationDistance = CurrentRotation.AngularDistance(DesiredRotation);
	const float MaxRotationDelta = TraceGrabConstants::MaxRotationSpeedInRadians * DeltaSeconds;
	const FQuat NewRotation = FQuat::Slerp(CurrentRotation, DesiredRotation, FMath::Min(MaxRotationDelta / RotationDistance, 1.0));
	
	GrabbedComponent->MoveComponent(MovementDelta, NewRotation, true);
	GrabbedObject->OnGrabbableMoved(MovementDelta.Length() / DeltaSeconds);
}

TObjectPtr<USceneComponent> UTraceGrabDevice::GetComponentToAttachTo() const
{
	return PlayerCharacter->GetCameraComponent();
}

void UTraceGrabDevice::OnSuccessfulRelease()
{
	Super::OnSuccessfulRelease();

	bIsPendingRelease = false;
	HeldThroughPortals.Empty();
}

FVector UTraceGrabDevice::GetDesiredGrabbedObjectLocation() const
{
	ensure(GrabbedObject);

	FVector DesiredLocation = OwnerComponent->GetComponentTransform().
	                                          TransformPosition(TraceGrabConstants::HeldObjectOffset);
	for (TWeakObjectPtr<APortal> WeakPassedPortal : HeldThroughPortals)
	{
		if (!WeakPassedPortal.IsValid())
		{
			UE_LOG(LogGrab, Error, TEXT("Reference to grabbed object's passed portal is invalid"));
			return DesiredLocation;
		}

		DesiredLocation = WeakPassedPortal->TeleportLocation(DesiredLocation);
	}

	return DesiredLocation;
}

void UTraceGrabDevice::OnActorTeleported(TObjectPtr<ITeleportable> Actor, TObjectPtr<APortal> SourcePortal,
                                         TObjectPtr<APortal> TargetPortal)
{
	if (!GrabbedObject)
	{
		return;
	}

	if (GrabbedObject->CastToGrabbableActor() == Actor->CastToTeleportableActor())
	{
		OnGrabbedObjectTeleported(SourcePortal, TargetPortal);
	}

	if (PlayerCharacter == Actor->CastToTeleportableActor())
	{
		OnOwnerCharacterTeleported(SourcePortal, TargetPortal);
	}
}

void UTraceGrabDevice::OnGrabbedObjectTeleported(TObjectPtr<APortal> SourcePortal, TObjectPtr<APortal> TargetPortal)
{
	const int32 PortalCount = HeldThroughPortals.Num();
	const APortal* LastPortal = PortalCount > 0 ? HeldThroughPortals[PortalCount - 1].Get() : nullptr;
	if (LastPortal == TargetPortal)
	{
		// Object was teleported back from the last portal, remove it from the list
		HeldThroughPortals.RemoveAt(PortalCount - 1);
	}
	else
	{
		// Object was teleported through a new portal, add it to the list
		HeldThroughPortals.Add(SourcePortal);
	}
}

void UTraceGrabDevice::OnOwnerCharacterTeleported(TObjectPtr<APortal> SourcePortal, TObjectPtr<APortal> TargetPortal)
{
	const int32 PortalCount = HeldThroughPortals.Num();
	const APortal* FirstPortal = PortalCount > 0 ? HeldThroughPortals[0].Get() : nullptr;
	if (FirstPortal == SourcePortal)
	{
		// Character went through the first of portals between it and grabbed object. Since we're no longer holding object through this portal, delete it from the array.
		HeldThroughPortals.RemoveAt(0);
	}
	else
	{
		// Character went through a different portal from the first one that we're holding object through so we need to add this new portal to the front of the list.
		HeldThroughPortals.Insert(TargetPortal, 0);
	}
}

bool UTraceGrabDevice::ShouldKeepHoldingObject() const
{
	const FVector OwnerLocation = OwnerComponent->GetComponentLocation();
	const FVector ObjectLocation = GrabbedObject->GetLocation();

	// get transformed-back point for each portal we're holding an object through
	FVector TransformedObjectLocation = ObjectLocation;
	TArray<TTuple<FVector, APortal*>> TransformedPointMap = {{ObjectLocation, nullptr}};
	for (int32 Index = HeldThroughPortals.Num() - 1; Index >= 0; --Index)
	{
		APortal* Portal = HeldThroughPortals[Index].Get();
		APortal* BackwardsPortal = Portal->GetConnectedPortal();
		TransformedObjectLocation = BackwardsPortal->TeleportLocation(TransformedObjectLocation);
		TransformedPointMap.Add({TransformedObjectLocation, Portal});
	}

	Algo::Reverse(TransformedPointMap);
	FVector StartPoint = OwnerLocation;

	// Check whether we're facing the grabbed object. First point is enough to determine that because angle between
	// direction to object and direction the owner is facing will stay the same after transformation via portals.
	const FVector ToFirstPointDir = (TransformedPointMap[0].Key - StartPoint).GetSafeNormal();
	if (ToFirstPointDir.Dot(OwnerComponent->GetComponentRotation().Vector()) < TraceGrabConstants::MinHoldDotProduct)
	{
		UE_LOG(LogGrab, Verbose, TEXT("Not facing grabbed object, dot: %.2f, dropping"),
		       ToFirstPointDir.Dot(OwnerComponent->GetComponentRotation().Vector()));
		return false;
	}

	float DistanceToObject = 0.f;
	for (const auto& Entry : TransformedPointMap)
	{
		const FVector& Point = Entry.Key;
		const APortal* Portal = Entry.Value;
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, Point, ECC_GrabObstruction))
		{
			DistanceToObject += FVector::Distance(StartPoint, HitResult.Location);
			if (HitResult.GetActor() != Portal)
			{
				UE_LOG(LogGrab, Verbose, TEXT("Object %s is blocking the view to grabbed object, dropping"), *HitResult.GetActor()->GetName())
				return false;
			}

			APortal* HitPortal = Cast<APortal>(HitResult.GetActor());
			StartPoint = HitPortal->TeleportLocation(HitResult.Location);
		}
		else
		{
			if (Portal)
			{
				UE_LOG(LogGrab, Verbose, TEXT("Trace to object failed to collide with portal when it was supposed to, dropping"));
				return false;
			}
			
			DistanceToObject += FVector::Distance(StartPoint, Point);
			break;
		}
	}

	if (DistanceToObject > TraceGrabConstants::MaxHoldDistance)
	{
		UE_LOG(LogGrab, Verbose, TEXT("Grabbed object is too far away, distance: %.2f, dropping"),
		       DistanceToObject);
		return false;
	}

	return true;
}

FQuat UTraceGrabDevice::GetDesiredGrabbedObjectRotation()
{
	FVector Location = GrabbedObject->GetLocation();
	for (int32 Index = HeldThroughPortals.Num() - 1; Index >= 0; --Index)
	{
		const APortal* BackwardsPortal = HeldThroughPortals[Index]->GetConnectedPortal();
		Location = BackwardsPortal->TeleportLocation(Location);
	}

	FQuat OwnerSpaceRotation = (Location - OwnerComponent->GetComponentLocation()).ToOrientationQuat();
	for (TWeakObjectPtr<APortal> Portal : HeldThroughPortals)
	{
		OwnerSpaceRotation = Portal->TeleportRotation(OwnerSpaceRotation);
	}
	return OwnerSpaceRotation;
}
