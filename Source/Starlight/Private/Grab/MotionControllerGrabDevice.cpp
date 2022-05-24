// Shadowhoof Games, 2022


#include "Grab/MotionControllerGrabDevice.h"

#include "Grab/Grabbable.h"
#include "DrawDebugHelpers.h"

namespace GrabConstants
{
	const float TraceRadius = 20.f;
}


void UMotionControllerGrabDevice::Initialize(TObjectPtr<USceneComponent> InOwnerComponent)
{
	Super::Initialize(InOwnerComponent);
	GrabTraceShape.SetSphere(GrabConstants::TraceRadius);
}

bool UMotionControllerGrabDevice::TryGrabbing()
{
	TArray<FHitResult> HitResults;
	const FVector TraceCenter = OwnerComponent->GetComponentLocation();
	GetWorld()->SweepMultiByChannel(HitResults, TraceCenter, TraceCenter, FQuat(), ECC_PhysicsBody, GrabTraceShape);
	DrawDebugSphere(GetWorld(), TraceCenter, GrabConstants::TraceRadius, 16, FColor::Yellow, false, 3);

	TObjectPtr<IGrabbable> ClosestGrabbable = nullptr;
	float ClosestDistSquared = FLT_MAX;
	for (const FHitResult& HitResult : HitResults)
	{
		if (!HitResult.bBlockingHit)
		{
			continue;
		}

		const TObjectPtr<IGrabbable> Grabbable = Cast<IGrabbable>(HitResult.GetActor());
		if (Grabbable)
		{
			const float DistSquared = Grabbable && FVector::DistSquared(TraceCenter, Grabbable->GetLocation());
			if (DistSquared < ClosestDistSquared)
			{
				ClosestGrabbable = Grabbable;
				ClosestDistSquared = DistSquared;
			}
		}
	}

	if (ClosestGrabbable)
	{
		return Grab(ClosestGrabbable);
	}

	return false;
}

TObjectPtr<USceneComponent> UMotionControllerGrabDevice::GetComponentToAttachTo() const
{
	return OwnerComponent;
}
