// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Grabbable.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogGrab, Log, All);


UINTERFACE()
class UGrabbable : public UInterface
{
	GENERATED_BODY()
};


/**
 *  Interface for objects which can be grabbed.
 */
class STARLIGHT_API IGrabbable
{
	GENERATED_BODY()

public:

	virtual void OnGrab();
	
	virtual void OnRelease();

	virtual bool IsGrabbed() const;
	
	virtual TObjectPtr<UPrimitiveComponent> GetComponentToGrab() const;

	virtual FVector GetLocation();

	TScriptInterface<IGrabbable> GetGrabbableScriptInterface();

	TObjectPtr<AActor> CastToGrabbableActor();
	TObjectPtr<const AActor> CastToGrabbableActor() const;

	virtual void OnGrabbableMoved(const float Speed);
	
};
