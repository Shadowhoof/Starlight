// Shadowhoof Games, 2022


#include "Grab/Grabbable.h"


void IGrabbable::OnGrab()
{
	if (const AActor* AsActor = Cast<AActor>(this))
	{
		UE_LOG(LogTemp, Log, TEXT("Object %s grabbed"), *AsActor->GetName());
	}
}

void IGrabbable::OnRelease()
{
	if (const AActor* AsActor = Cast<AActor>(this))
	{
		UE_LOG(LogTemp, Log, TEXT("Object %s released"), *AsActor->GetName());
	}
}

TObjectPtr<UPrimitiveComponent> IGrabbable::GetAttachComponent() const
{
	return nullptr;
}

FVector IGrabbable::GetLocation()
{
	const TObjectPtr<UPrimitiveComponent> AttachComponent = GetAttachComponent();
	return AttachComponent ? AttachComponent->GetComponentLocation() : FVector::ZeroVector;
}
