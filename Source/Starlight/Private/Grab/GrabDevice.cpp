// Shadowhoof Games, 2022


#include "Grab/GrabDevice.h"

#include "Grab/Grabbable.h"
#include "Statics/StarlightMacros.h"


TObjectPtr<IGrabbable> UGrabDevice::GetGrabbedObject() const
{
	return GrabbedObject.GetInterface();
}

bool UGrabDevice::TryGrabbing()
{
	return false;
}

bool UGrabDevice::Grab(TObjectPtr<IGrabbable> ObjectToGrab)
{
	if (GrabbedObject)
	{
		return false;
	}

	const TObjectPtr<UPrimitiveComponent> GrabbedComponent = ObjectToGrab->GetAttachComponent();
	GrabbedComponent->SetSimulatePhysics(false);
	const bool bIsGrabbed = GrabbedComponent->AttachToComponent(GetComponentToAttachTo(),
	                                                            {EAttachmentRule::KeepWorld, false});
	UE_LOG(LogGrab, Verbose, TEXT("Trying to grab actor %s, result: %s"), *GrabbedComponent->GetOwner()->GetName(),
	       BOOL_TO_STRING(bIsGrabbed));

	if (bIsGrabbed)
	{
		OnSuccessfulGrab(ObjectToGrab);
	}

	return bIsGrabbed;
}

void UGrabDevice::OnSuccessfulGrab(TObjectPtr<IGrabbable> ObjectToGrab)
{
	ensure(ObjectToGrab && !GrabbedObject);
	ObjectToGrab->OnGrab();
	GrabbedObject = ObjectToGrab->GetGrabbableScriptInterface();
}

void UGrabDevice::OnSuccessfulRelease()
{
	ensure(GrabbedObject);
	GrabbedObject->OnRelease();
	GrabbedObject = nullptr;
}

void UGrabDevice::Release()
{
	if (GrabbedObject)
	{
		const TObjectPtr<UPrimitiveComponent> GrabbedComponent = GrabbedObject->GetAttachComponent();
		GrabbedComponent->SetSimulatePhysics(true);
		GrabbedComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		OnSuccessfulRelease();
	}
}

TObjectPtr<USceneComponent> UGrabDevice::GetComponentToAttachTo() const
{
	return nullptr;
}
