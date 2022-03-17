﻿// Shadowhoof Games, 2022


#include "Grab/TraceGrabDevice.h"

#include "Core/StarlightCharacter.h"
#include "Camera/CameraComponent.h"
#include "Grab/Grabbable.h"

namespace TraceGrabConstants
{
	const float GrabRange = 500.f;
}


bool UTraceGrabDevice::TryGrabbing()
{
	FHitResult HitResult;
	const FVector StartPoint = PlayerCharacter->GetPawnViewLocation();
	const FVector EndPoint = StartPoint + PlayerCharacter->GetControlRotation().Vector() * TraceGrabConstants::GrabRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerCharacter);
	GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECC_PhysicsBody, QueryParams);

	if (HitResult.IsValidBlockingHit())
	{
		if (TObjectPtr<IGrabbable> Grabbable = Cast<IGrabbable>(HitResult.GetActor()))
		{
			UE_LOG(LogGrab, VeryVerbose, TEXT("Grab trace has hit grabbable actor %s"), *HitResult.GetActor()->GetName());
			return Grab(Grabbable);
		}
		else
		{
			UE_LOG(LogGrab, VeryVerbose, TEXT("Grab trace has hit non-grabbable actor %s"), *HitResult.GetActor()->GetName());
		}
	}

	return false;
}

void UTraceGrabDevice::Initialize(TObjectPtr<AStarlightCharacter> Character)
{
	PlayerCharacter = Character;
}

TObjectPtr<USceneComponent> UTraceGrabDevice::GetComponentToAttachTo() const
{
	return PlayerCharacter->GetCameraComponent();
}
