// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Teleportable.generated.h"

class APortalSurface;
class APortal;
class ATeleportableCopy;

UINTERFACE()
class UTeleportable : public UInterface
{
	GENERATED_BODY()
};


/** Objects implementing this interface can be teleported through a portal */
class STARLIGHT_API ITeleportable
{
	GENERATED_BODY()

public:
	
	/** Teleports an object from source portal to target portal */
	virtual void Teleport(TObjectPtr<APortal> SourcePortal, TObjectPtr<APortal> TargetPortal);
	
	/** Enables collision with provided portal surface.  */
	virtual void EnableCollisionWith(TObjectPtr<APortalSurface> PortalSurface);

	/**
	 * Disables collision with provided portal surface. Disables physics collision if it's a physics object or adds
	 * objects to their respective IgnoreMoveActors arrays otherwise.
	 */
	virtual void DisableCollisionWith(TObjectPtr<APortalSurface> PortalSurface);

	TObjectPtr<AActor> CastToTeleportableActor();
	TObjectPtr<const AActor> CastToTeleportableActor() const;

	/**
	 * Called when portal is nearby and object can potentially be teleported soon. Called only when there is a
	 * connected portal present.
	 */
	virtual void OnOverlapWithPortalBegin(TObjectPtr<APortal> Portal);

	/** Called when character is no longer within portal's teleport detection range */
	virtual void OnOverlapWithPortalEnd(TObjectPtr<APortal> Portal);

	TScriptInterface<ITeleportable> GetTeleportableScriptInterface();

	/** Creates copy of this object that will mirror the original object on the other side of the portal */
	virtual TObjectPtr<ATeleportableCopy> CreatePortalCopy(const FTransform& SpawnTransform, TObjectPtr<APortal> OwnerPortal, TObjectPtr<APortal> OtherPortal, TObjectPtr<ITeleportable> ParentActor);

	virtual TObjectPtr<UPrimitiveComponent> GetCollisionComponent() const;
	
	virtual void GetTeleportVelocity(FVector& LinearVelocity, FVector& AngularVelocity) const;
	virtual void SetTeleportVelocity(const FVector& LinearVelocity, const FVector& AngularVelocity);

	virtual void OnTeleportableMoved();
};
