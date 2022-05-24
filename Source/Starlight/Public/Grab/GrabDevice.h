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
	virtual void Initialize(TObjectPtr<USceneComponent> InOwnerComponent);
	
	TObjectPtr<IGrabbable> GetGrabbedObject() const;

	virtual bool TryGrabbing();
	
	/** Tries to grab provided object */
	virtual bool Grab(TObjectPtr<IGrabbable> ObjectToGrab);

	/** Releases grabbed object if any is grabbed */
	virtual void Release();

	virtual void Tick(const float DeltaSeconds);
	
protected:

	UPROPERTY()
	TScriptInterface<IGrabbable> GrabbedObject;

	UPROPERTY()
	TObjectPtr<USceneComponent> OwnerComponent;
	
	virtual void OnSuccessfulGrab(TObjectPtr<IGrabbable> ObjectToGrab);

	virtual void OnSuccessfulRelease();
	
	virtual TObjectPtr<USceneComponent> GetComponentToAttachTo() const;
	
};
