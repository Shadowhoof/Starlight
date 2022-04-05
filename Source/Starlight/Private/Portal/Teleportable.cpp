// Shadowhoof Games, 2022


#include "Portal/Teleportable.h"

#include "Portal/Portal.h"
#include "Portal/PortalConstants.h"
#include "Portal/PortalSurface.h"
#include "Statics/StarlightStatics.h"


bool ITeleportable::Teleport(const FVector& Location, const FRotator& Rotation)
{
	const FVector RelativeVelocity = GetRotation().UnrotateVector(GetVelocity());
	
	UE_LOG(LogPortal, VeryVerbose, TEXT("Rot before tp: %s, Vel before tp: %s, Relative vel: %s"),
	       *FRotator().ToCompactString(), *FRotator().ToCompactString(), *FRotator().ToCompactString());
	
	const bool bHasTeleported = SetTeleportLocationAndRotation(Location, Rotation);
	if (bHasTeleported)
	{
		SetVelocity(GetRotation().RotateVector(RelativeVelocity));
		
		UE_LOG(LogPortal, VeryVerbose, TEXT("Rot after tp: %s, Vel after tp: %s"),
		       *GetRotation().ToCompactString(), *GetVelocity().ToCompactString());
	}

	return bHasTeleported;
}

void ITeleportable::EnableCollisionWith(TObjectPtr<APortalSurface> PortalSurface)
{
	const TObjectPtr<UPrimitiveComponent> CollisionComponent = GetCollisionComponent();
	if (CollisionComponent)
	{
		UStarlightStatics::EnableCollisionBetween(PortalSurface->GetCollisionComponent(), CollisionComponent);
	}
	else
	{
		UE_LOG(LogPortal, Warning, TEXT("No collision component set up for %s"), *CastToTeleportableActor()->GetName())
	}
}

void ITeleportable::DisableCollisionWith(TObjectPtr<APortalSurface> PortalSurface)
{
	const TObjectPtr<UPrimitiveComponent> CollisionComponent = GetCollisionComponent();
	if (CollisionComponent)
	{
		UStarlightStatics::DisableCollisionBetween(PortalSurface->GetCollisionComponent(), CollisionComponent);
	}
	else
	{
		UE_LOG(LogPortal, Warning, TEXT("No collision component set up for %s"), *CastToTeleportableActor()->GetName())
	}
}

TObjectPtr<AActor> ITeleportable::CastToTeleportableActor()
{
	return Cast<AActor>(this);
}

TObjectPtr<const AActor> ITeleportable::CastToTeleportableActor() const
{
	return Cast<AActor>(this);
}

void ITeleportable::OnOverlapWithPortalBegin(TObjectPtr<APortal> Portal)
{
	UE_LOG(LogPortal, Verbose, TEXT("Portal %s is now overlapping with %s"), *Portal->GetName(), *CastToTeleportableActor()->GetName());
	DisableCollisionWith(Portal->GetPortalSurface());
}

void ITeleportable::OnOverlapWithPortalEnd(TObjectPtr<APortal> Portal)
{
	UE_LOG(LogPortal, Verbose, TEXT("Portal %s is no longer overlapping with %s"), *Portal->GetName(), *CastToTeleportableActor()->GetName());
	EnableCollisionWith(Portal->GetPortalSurface());
}

TScriptInterface<ITeleportable> ITeleportable::GetTeleportableScriptInterface()
{
	TScriptInterface<ITeleportable> ScriptInterface;
	ScriptInterface.SetInterface(this);
	ScriptInterface.SetObject(CastToTeleportableActor());
	return ScriptInterface;
}

FRotator ITeleportable::GetTeleportRotation()
{
	UE_LOG(LogPortal, Warning, TEXT("ITeleportable::GetTeleportRotation is not overriden"));
	return FRotator::ZeroRotator;
}

bool ITeleportable::SetTeleportLocationAndRotation(const FVector& Location, const FRotator& Rotation)
{
	return false;
}

TObjectPtr<UPrimitiveComponent> ITeleportable::GetCollisionComponent() const
{
	return nullptr;
}

FRotator ITeleportable::GetRotation() const
{
	return CastToTeleportableActor()->GetActorRotation();
}

FVector ITeleportable::GetVelocity() const
{
	return FVector::ZeroVector;
}

void ITeleportable::SetVelocity(const FVector& Velocity)
{
}
