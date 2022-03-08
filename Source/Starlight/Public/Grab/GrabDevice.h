// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GrabDevice.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGrab, Log, All);

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

	UPROPERTY(Transient)
	TObjectPtr<IGrabbable> GrabbedObject = nullptr;

	void OnSuccessfulGrab(TObjectPtr<IGrabbable> ObjectToGrab);

	void OnSuccessfulRelease();
	
	virtual TObjectPtr<USceneComponent> GetComponentToAttachTo() const;
	
};
