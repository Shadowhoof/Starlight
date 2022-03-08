// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GrabDevice.h"
#include "MotionControllerGrabDevice.generated.h"


class UMotionControllerComponent;

/**
 *  Grab device which is using motion controllers to grab things.
 */
UCLASS()
class STARLIGHT_API UMotionControllerGrabDevice : public UGrabDevice
{
	GENERATED_BODY()

public:

	void Initialize(TObjectPtr<UMotionControllerComponent> Controller);

	virtual bool TryGrabbing() override;

protected:

	virtual TObjectPtr<USceneComponent> GetComponentToAttachTo() const override;
	
private:

	UPROPERTY(Transient)
	TObjectPtr<UMotionControllerComponent> MotionController;

	FCollisionShape GrabTraceShape;
	
};
