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
