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

inline ECollisionChannel GetInnerObjectTypeForPortalType(EPortalType PortalType)
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
