// Shadowhoof Games, 2022


#include "Statics/StarlightStatics.h"

#include "IXRTrackingSystem.h"
#include "PBDRigidsSolver.h"
#include "Chaos/Collision/CollisionConstraintFlags.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

bool UStarlightStatics::IsHMDActive()
{
	return GEngine->XRSystem && GEngine->XRSystem->IsHeadTrackingAllowed();
}

void UStarlightStatics::EnableCollisionBetween(TObjectPtr<UPrimitiveComponent> First,
                                               TObjectPtr<UPrimitiveComponent> Second)
{
	// enable physics interaction
	Chaos::FIgnoreCollisionManager& CollisionManager = GetIgnoreCollisionManager(First);
	const Chaos::FUniqueIdx FirstHandle = GetPhysicsHandleID(First);
	const Chaos::FUniqueIdx SecondHandle = GetPhysicsHandleID(Second);
	CollisionManager.RemoveIgnoreCollisionsFor(FirstHandle, SecondHandle);
	CollisionManager.RemoveIgnoreCollisionsFor(SecondHandle, FirstHandle);

	// enable non-physics interaction
	First->IgnoreComponentWhenMoving(Second, false);
	Second->IgnoreComponentWhenMoving(First, false);
}

void UStarlightStatics::DisableCollisionBetween(TObjectPtr<UPrimitiveComponent> First,
                                                TObjectPtr<UPrimitiveComponent> Second)
{
	// disable physics interaction
	Chaos::FPBDRigidParticle* FirstParticle = First->BodyInstance.ActorHandle->GetRigidParticleUnsafe();
	FirstParticle->AddCollisionConstraintFlag(Chaos::ECollisionConstraintFlags::CCF_BroadPhaseIgnoreCollisions);
	Chaos::FPBDRigidParticle* SecondParticle = Second->BodyInstance.ActorHandle->GetRigidParticleUnsafe();
	SecondParticle->AddCollisionConstraintFlag(Chaos::ECollisionConstraintFlags::CCF_BroadPhaseIgnoreCollisions);

	Chaos::FIgnoreCollisionManager& CollisionManager = GetIgnoreCollisionManager(First);
	const Chaos::FUniqueIdx FirstHandle = GetPhysicsHandleID(First);
	const Chaos::FUniqueIdx SecondHandle = GetPhysicsHandleID(Second);
	CollisionManager.AddIgnoreCollisionsFor(FirstHandle, SecondHandle);
	CollisionManager.AddIgnoreCollisionsFor(SecondHandle, FirstHandle);

	// disable non-physics interaction	
	First->IgnoreComponentWhenMoving(Second, true);
	Second->IgnoreComponentWhenMoving(First, true);
}

bool UStarlightStatics::IsPhysicsCollisionIgnored(TObjectPtr<UPrimitiveComponent> First,
                                                  TObjectPtr<UPrimitiveComponent> Second)
{
	Chaos::FIgnoreCollisionManager& CollisionManager = GetIgnoreCollisionManager(First);
	return CollisionManager.IgnoresCollision(GetPhysicsHandleID(First), GetPhysicsHandleID(Second));
}

Chaos::FIgnoreCollisionManager& UStarlightStatics::GetIgnoreCollisionManager(TObjectPtr<UPrimitiveComponent> Component)
{
	const FPhysicsActorHandle Handle = Component->BodyInstance.ActorHandle;
	Chaos::FPhysicsSolver* Solver = Handle->GetSolver<Chaos::FPhysicsSolver>();
	return Solver->GetEvolution()->GetBroadPhase().GetIgnoreCollisionManager();
}

Chaos::FUniqueIdx UStarlightStatics::GetPhysicsHandleID(TObjectPtr<UPrimitiveComponent> Component)
{
	return Component->BodyInstance.ActorHandle->GetGameThreadAPI().UniqueIdx();
}

bool UStarlightStatics::ComponentEncroachesBlockingGeometry(TObjectPtr<AActor> Actor,
                                                            TObjectPtr<UPrimitiveComponent> Component,
                                                            const FVector& Location, const FRotator& Rotation,
                                                            const TArray<TObjectPtr<AActor>>& IgnoredActors,
                                                            FVector& OutAdjustment)
{
	const FQuat QuatRotation = FQuat(Rotation);
	const ECollisionChannel ObjectType = Component->GetCollisionObjectType();
	OutAdjustment = FVector::ZeroVector;
	
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ComponentEncroachesBlockingGeometry), false, Actor);
	FCollisionResponseParams ResponseParams;
	Component->InitSweepCollisionParams(Params, ResponseParams);
	Params.AddIgnoredActors(IgnoredActors);
	bool bFoundBlockingHit = Actor->GetWorld()->OverlapMultiByChannel(Overlaps, Location, QuatRotation,
	                                                                  ObjectType, Component->GetCollisionShape(),
	                                                                  Params, ResponseParams);

	// if encroaching, add up all the MTDs of overlapping shapes
	FMTDResult MTDResult;
	uint32 NumBlockingHits = 0;
	OutAdjustment = FVector::ZeroVector;
	for (int32 HitIdx = 0; HitIdx < Overlaps.Num(); HitIdx++)
	{
		UPrimitiveComponent* const OverlapComponent = Overlaps[HitIdx].Component.Get();
		// first determine closest impact point along each axis
		if (OverlapComponent && OverlapComponent->GetCollisionResponseToChannel(ObjectType) == ECR_Block)
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
