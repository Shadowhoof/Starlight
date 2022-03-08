// Shadowhoof Games, 2022


#include "Grab/GrabDevice.h"

#include "Grab/Grabbable.h"

DEFINE_LOG_CATEGORY(LogGrab);


TObjectPtr<IGrabbable> UGrabDevice::GetGrabbedObject() const
{
	return GrabbedObject;
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
	const bool bIsGrabbed = GrabbedComponent->AttachToComponent(GetComponentToAttachTo(), {EAttachmentRule::KeepWorld, false});
	
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
	GrabbedObject = ObjectToGrab;
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
