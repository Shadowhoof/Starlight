// Shadowhoof Games, 2022


#include "Grab/GrabDevice.h"

#include "Core/StarlightCharacter.h"
#include "Grab/Grabbable.h"
#include "Statics/StarlightMacros.h"


void UGrabDevice::Initialize(TObjectPtr<USceneComponent> InOwnerComponent)
{
	if (!InOwnerComponent)
	{
		UE_LOG(LogGrab, Error, TEXT("Owner component for UGrabDevice is nullptr"));
	}
	
	OwnerComponent = InOwnerComponent;
	PlayerCharacter = Cast<AStarlightCharacter>(InOwnerComponent->GetOwner());
}

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
	
	UPrimitiveComponent* GrabbedComponent = ObjectToGrab->GetComponentToGrab();
	GrabbedComponent->SetSimulatePhysics(false);
	OnSuccessfulGrab(ObjectToGrab);
	return true;
}

void UGrabDevice::OnSuccessfulGrab(TObjectPtr<IGrabbable> ObjectToGrab)
{
	ensure(ObjectToGrab && !GrabbedObject);
	ObjectToGrab->OnGrab();
	GrabbedObject = ObjectToGrab->GetGrabbableScriptInterface();
	PlayerCharacter->OnObjectGrabbed(ObjectToGrab);
}

void UGrabDevice::OnSuccessfulRelease()
{
	ensure(GrabbedObject);
	PlayerCharacter->OnObjectReleased(GrabbedObject.GetInterface());
	GrabbedObject->OnRelease();
	GrabbedObject = nullptr;
}

void UGrabDevice::Release()
{
	if (GrabbedObject)
	{
		UPrimitiveComponent* GrabbedComponent = GrabbedObject->GetComponentToGrab();
		GrabbedComponent->SetSimulatePhysics(true);
		GrabbedComponent->WakeRigidBody();
		OnSuccessfulRelease();
	}
}

void UGrabDevice::Tick(const float DeltaSeconds)
{
}

TObjectPtr<USceneComponent> UGrabDevice::GetComponentToAttachTo() const
{
	return nullptr;
}
