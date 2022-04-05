// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GrabDevice.generated.h"


class IGrabbable;

UCLASS(Abstract)
class STARLIGHT_API UGrabDevice : public UObject
{
	GENERATED_BODY()

public:
	TObjectPtr<IGrabbable> GetGrabbedObject() const;

	virtual bool TryGrabbing();
	
	/** Tries to grab provided object */
	virtual bool Grab(TObjectPtr<IGrabbable> ObjectToGrab);

	/** Releases grabbed object if any is grabbed */
	virtual void Release();

protected:

	UPROPERTY()
	TScriptInterface<IGrabbable> GrabbedObject;

	void OnSuccessfulGrab(TObjectPtr<IGrabbable> ObjectToGrab);

	void OnSuccessfulRelease();
	
	virtual TObjectPtr<USceneComponent> GetComponentToAttachTo() const;
	
};
