// Shadowhoof Games, 2022


#include "Portal/PortalStatics.h"

#include "Portal/Portal.h"
#include "Portal/PortalSurface.h"

namespace Constants
{
	const int32 MaxTraceIterations = 10;
}


bool UPortalStatics::TransformPositionThroughPortal(TObjectPtr<UObject> WorldContextObject,
                                                    const FTransform& Transform,
                                                    const FVector& LocalPosition,
                                                    FVector& OutPosition)
{
	const FVector OldLocation = Transform.GetLocation();
	const FVector NewLocation = Transform.TransformPosition(LocalPosition);
	// preliminary result
	OutPosition = NewLocation;

	UWorld* World = WorldContextObject->GetWorld();
	FHitResult HitResult;
	World->LineTraceSingleByObjectType(HitResult, OldLocation, NewLocation, {ECC_PortalBody});
	if (!HitResult.IsValidBlockingHit())
	{
		return false;
	}

	APortal* HitPortal = Cast<APortal>(HitResult.GetActor());
	if (!HitPortal)
	{
		UE_LOG(LogPortal, Warning, TEXT("Blocking hit via ECC_PortalBody but it's not a portal, hit actor: %s"),
		       *HitResult.GetActor()->GetName());
		return false;
	}

	APortal* OtherPortal = HitPortal->GetConnectedPortal();
	if (!OtherPortal)
	{
		return false;
	}

	OutPosition = HitPortal->TeleportLocation(NewLocation);
	return true;
}

bool UPortalStatics::LineTraceThroughPortal(TObjectPtr<UWorld> World,
                                            FHitResult& OutHitResult,
                                            FVector Start,
                                            FVector End,
                                            ECollisionChannel Channel,
                                            TArray<TObjectPtr<APortal>>* CrossedPortals,
                                            FCollisionQueryParams QueryParams,
                                            FCollisionResponseParams ResponseParams)
{
	int32 Counter = 0;
	const FVector OriginalStart = Start;
	const FVector OriginalEnd = End;

	while (Counter < Constants::MaxTraceIterations)
	{
		const bool bBlockingHit = World->LineTraceSingleByChannel(OutHitResult, Start, End, Channel, QueryParams, ResponseParams);
		if (!bBlockingHit)
		{
			return false;
		}

		if (OutHitResult.GetComponent()->GetCollisionObjectType() != ECC_PortalBody)
		{
			return true;
		}

		APortal* HitPortal = Cast<APortal>(OutHitResult.GetActor());
		if (!HitPortal)
		{
			UE_LOG(LogPortal, Warning, TEXT("Blocking hit via ECC_PortalBody but it's not a portal, hit actor: %s"),
			       *OutHitResult.GetActor()->GetName());
			return true;
		}

		const APortal* OtherPortal = HitPortal->GetConnectedPortal();
		if (!OtherPortal)
		{
			return true;
		}

		TArray<TObjectPtr<AActor>> PortalSurfaceActors;
		OtherPortal->GetPortalSurface()->GetCollisionActors(PortalSurfaceActors);
		
		QueryParams.ClearIgnoredActors();
		QueryParams.AddIgnoredActors(PortalSurfaceActors);
		QueryParams.AddIgnoredActor(OtherPortal);
		
		Start = HitPortal->TeleportLocation(OutHitResult.Location);
		End = HitPortal->TeleportLocation(End);

		if (CrossedPortals)
		{
			CrossedPortals->Add(HitPortal);
		}
		
		Counter++;
	}

	UE_LOG(LogPortal, Warning,
	       TEXT(
		       "UPortalStatics::LineTraceThroughPortal trace loop counter has exceeded max loop count of %d (start: %s, end: %s)"
	       ), Constants::MaxTraceIterations, *OriginalStart.ToCompactString(), *OriginalEnd.ToCompactString());
	return false;
}

EPortalType UPortalStatics::GetOtherPortalType(EPortalType PortalType)
{
	return PortalType == EPortalType::First ? EPortalType::Second : EPortalType::First;
}

ECollisionChannel UPortalStatics::GetCopyObjectType(EPortalType PortalType)
{
	return PortalType == EPortalType::First ? ECC_FirstPortalCopy : ECC_SecondPortalCopy;
}

ECollisionChannel UPortalStatics::GetOpposingCopyObjectType(EPortalType PortalType)
{
	return PortalType == EPortalType::First ? ECC_SecondPortalCopy : ECC_FirstPortalCopy;
}

ECollisionChannel UPortalStatics::GetInnerObjectTypeForPortalType(EPortalType PortalType)
{
	return PortalType == EPortalType::First ? ECC_WithinFirstPortal : ECC_SecondPortalCopy;
}

ECollisionChannel UPortalStatics::GetObjectTypeOnOverlapBegin(TObjectPtr<ITeleportable> Teleportable, EPortalType PortalType)
{
	ECollisionChannel CurrentObjectType = Teleportable->GetCollisionComponent()->GetCollisionObjectType();
	if (PortalType == EPortalType::First)
	{
		return CurrentObjectType == ECC_WithinSecondPortal ? ECC_WithinBothPortals : ECC_WithinFirstPortal;
	}
	
	return CurrentObjectType == ECC_WithinFirstPortal ? ECC_WithinBothPortals : ECC_WithinSecondPortal;
}

ECollisionChannel UPortalStatics::GetObjectTypeOnOverlapEnd(TObjectPtr<ITeleportable> Teleportable, EPortalType PortalType)
{
	ECollisionChannel CurrentObjectType = Teleportable->GetCollisionComponent()->GetCollisionObjectType();
	if (CurrentObjectType == ECC_WithinBothPortals)
	{
		return PortalType == EPortalType::First ? ECC_WithinSecondPortal : ECC_WithinFirstPortal;
	}

	return Teleportable->GetTeleportableBaseObjectType();
}

bool CanComponentEncroachTeleportingActor(TObjectPtr<UPrimitiveComponent> OverlapComponent,
										  ECollisionChannel TeleportingObjectType,
										  const FVector& PortalLocation,
										  const FVector& PortalNormal)
{
	if (!OverlapComponent || OverlapComponent->GetCollisionResponseToChannel(TeleportingObjectType) != ECR_Block)
	{
		return false;
	}
	
	if (TeleportingCopyTypes.Contains(OverlapComponent->GetCollisionObjectType()))
	{
		return true;
	}
	
	const FVector PortalToComponentDir = OverlapComponent->GetComponentLocation() - PortalLocation;
	return PortalToComponentDir.Dot(PortalNormal) >= 0.f;
}

bool UPortalStatics::ComponentEncroachesBlockingGeometryOnTeleport(TObjectPtr<AActor> Actor,
                                                            TObjectPtr<UPrimitiveComponent> Component,
                                                            const FVector& Location, const FRotator& Rotation,
                                                            const TArray<TObjectPtr<AActor>>& IgnoredActors,
                                                            FVector& OutAdjustment, TObjectPtr<APortal> TargetPortal,
                                                            ECollisionChannel ObjectType)
{
	const FQuat QuatRotation = FQuat(Rotation);
	if (ObjectType == ECC_MAX)
	{
		ObjectType = Component->GetCollisionObjectType(); 
	}
	OutAdjustment = FVector::ZeroVector;
	
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ComponentEncroachesBlockingGeometry), false, Actor);
	FCollisionResponseParams ResponseParams;
	Component->InitSweepCollisionParams(Params, ResponseParams);
	Params.AddIgnoredActors(IgnoredActors);
	bool bFoundBlockingHit = Actor->GetWorld()->OverlapMultiByChannel(Overlaps, Location, QuatRotation,
	                                                                  ObjectType, Component->GetCollisionShape(),
	                                                                  Params, ResponseParams);

	const FVector PortalLocation = TargetPortal->GetActorLocation();
	const FVector PortalNormal = TargetPortal->GetActorForwardVector();
	
	// if encroaching, add up all the MTDs of overlapping shapes
	FMTDResult MTDResult;
	uint32 NumBlockingHits = 0;
	OutAdjustment = FVector::ZeroVector;
	for (int32 HitIdx = 0; HitIdx < Overlaps.Num(); HitIdx++)
	{
		UPrimitiveComponent* const OverlapComponent = Overlaps[HitIdx].Component.Get();
		// first determine closest impact point along each axis
		if (CanComponentEncroachTeleportingActor(OverlapComponent, ObjectType, PortalLocation, PortalNormal))
		{
			NumBlockingHits++;
			FCollisionShape const NonShrunkenCollisionShape = Component->GetCollisionShape();
			const FBodyInstance* OverlapBodyInstance = OverlapComponent->GetBodyInstance(NAME_None, true, Overlaps[HitIdx].ItemIndex);
			bool bSuccess = OverlapBodyInstance && OverlapBodyInstance->OverlapTest(Location, QuatRotation, NonShrunkenCollisionShape, &MTDResult);
			if (bSuccess)
			{
				OutAdjustment += MTDResult.Direction * MTDResult.Distance;
			}
			else
			{
				// It's not safe to use a partial result, that could push us out to an invalid location (like the other side of a wall).
				OutAdjustment = FVector::ZeroVector;
				return true;
			}
		}
	}

	// See if we chose to invalidate all of our supposed "blocking hits".
	if (NumBlockingHits == 0)
	{
		OutAdjustment = FVector::ZeroVector;
		bFoundBlockingHit = false;
	}

	return bFoundBlockingHit;
}
