// Shadowhoof Games, 2022


#include "Portal/Teleportable.h"

#include "Portal/Portal.h"
#include "Portal/PortalConstants.h"
#include "Portal/PortalSurface.h"
#include "Statics/StarlightStatics.h"


void ITeleportable::Teleport(TObjectPtr<APortal> SourcePortal, TObjectPtr<APortal> TargetPortal)
{
	AActor* AsActor = CastToTeleportableActor();
	const FVector NewLocation = SourcePortal->TeleportLocation(AsActor->GetActorLocation());
	const FRotator NewRotation = SourcePortal->TeleportRotation(AsActor->GetActorQuat());
	const FVector NewVelocity = SourcePortal->TeleportVelocity(GetVelocity());

	const bool bHasTeleported = AsActor->TeleportTo(NewLocation, NewRotation, false, true);
	if (!bHasTeleported)
	{
		UE_LOG(LogPortal, Error, TEXT("Actor %s failed to teleport"), *AsActor->GetName());
		return;
	}
	
	SetVelocity(NewVelocity);
}

void ITeleportable::EnableCollisionWith(TObjectPtr<APortalSurface> PortalSurface)
{
	const TObjectPtr<UPrimitiveComponent> TeleportableComponent = GetCollisionComponent();
	if (TeleportableComponent)
	{
		TArray<TObjectPtr<UPrimitiveComponent>> SurfaceCollisionComponents;
		PortalSurface->GetCollisionComponents(SurfaceCollisionComponents);
		for (UPrimitiveComponent* SurfaceComponent : SurfaceCollisionComponents)
		{
			UStarlightStatics::EnableCollisionBetween(SurfaceComponent, TeleportableComponent);
		}
	}
	else
	{
		UE_LOG(LogPortal, Warning, TEXT("No collision component set up for %s"), *CastToTeleportableActor()->GetName())
	}
}

void ITeleportable::DisableCollisionWith(TObjectPtr<APortalSurface> PortalSurface)
{
	const TObjectPtr<UPrimitiveComponent> TeleportableComponent = GetCollisionComponent();
	if (TeleportableComponent)
	{
		TArray<TObjectPtr<UPrimitiveComponent>> SurfaceCollisionComponents;
		PortalSurface->GetCollisionComponents(SurfaceCollisionComponents);
		for (UPrimitiveComponent* SurfaceComponent : SurfaceCollisionComponents)
		{
			UStarlightStatics::DisableCollisionBetween(SurfaceComponent, TeleportableComponent);
		}
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

	const ECollisionChannel ObjectTypeToBlock = GetOpposingCopyObjectType(Portal->GetPortalType());
	GetCollisionComponent()->SetCollisionResponseToChannel(ObjectTypeToBlock, ECR_Block);
}

void ITeleportable::OnOverlapWithPortalEnd(TObjectPtr<APortal> Portal)
{
	UE_LOG(LogPortal, Verbose, TEXT("Portal %s is no longer overlapping with %s"), *Portal->GetName(), *CastToTeleportableActor()->GetName());
	EnableCollisionWith(Portal->GetPortalSurface());

	const ECollisionChannel ObjectTypeToIgnore = GetOpposingCopyObjectType(Portal->GetPortalType());
	GetCollisionComponent()->SetCollisionResponseToChannel(ObjectTypeToIgnore, ECR_Ignore);
}

TScriptInterface<ITeleportable> ITeleportable::GetTeleportableScriptInterface()
{
	TScriptInterface<ITeleportable> ScriptInterface;
	ScriptInterface.SetInterface(this);
	ScriptInterface.SetObject(CastToTeleportableActor());
	return ScriptInterface;
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

TObjectPtr<ATeleportableCopy> ITeleportable::CreatePortalCopy(const FTransform& SpawnTransform,
                                                              TObjectPtr<APortal> Portal, TObjectPtr<AActor> ParentActor)
{
	UE_LOG(LogPortal, Error, TEXT("CreateCopy() is not implemented for %s"), *CastToTeleportableActor()->GetClass()->GetName());
	return nullptr;
}
