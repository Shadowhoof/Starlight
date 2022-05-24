// Shadowhoof Games, 2022


#include "Grab/Grabbable.h"


DEFINE_LOG_CATEGORY(LogGrab);


void IGrabbable::OnGrab()
{
	UE_LOG(LogGrab, Verbose, TEXT("Object %s grabbed"), *CastToGrabbableActor()->GetName());
}

void IGrabbable::OnRelease()
{
	UE_LOG(LogGrab, Verbose, TEXT("Object %s released"), *CastToGrabbableActor()->GetName());
}

bool IGrabbable::IsGrabbed() const
{
	return false;
}

TObjectPtr<UPrimitiveComponent> IGrabbable::GetComponentToGrab() const
{
	return nullptr;
}

FVector IGrabbable::GetLocation()
{
	const TObjectPtr<UPrimitiveComponent> ComponentToGrab = GetComponentToGrab();
	return ComponentToGrab ? ComponentToGrab->GetComponentLocation() : FVector::ZeroVector;
}

TScriptInterface<IGrabbable> IGrabbable::GetGrabbableScriptInterface()
{
	TScriptInterface<IGrabbable> ScriptInterface;
	ScriptInterface.SetInterface(this);
	ScriptInterface.SetObject(CastToGrabbableActor());
	return ScriptInterface;
}

TObjectPtr<AActor> IGrabbable::CastToGrabbableActor()
{
	return Cast<AActor>(this);
}

TObjectPtr<const AActor> IGrabbable::CastToGrabbableActor() const
{
	return Cast<AActor>(this);
}

void IGrabbable::OnGrabbableMoved(const float Speed)
{
}
