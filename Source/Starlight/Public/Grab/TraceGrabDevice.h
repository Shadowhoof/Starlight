// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GrabDevice.h"
#include "TraceGrabDevice.generated.h"

class AStarlightCharacter;
class APortal;
class ITeleportable;

/**
 *  Grab device which uses ray cast to figure out what to grab
 */
UCLASS()
class STARLIGHT_API UTraceGrabDevice : public UGrabDevice
{
	GENERATED_BODY()

public:

	virtual bool TryGrabbing() override;

	virtual void Initialize(TObjectPtr<USceneComponent> InOwnerComponent) override;

	virtual void Tick(const float DeltaSeconds) override;
	
protected:

	virtual TObjectPtr<USceneComponent> GetComponentToAttachTo() const override;

	virtual void OnSuccessfulRelease() override;

private:

	/** Portals between trace device and grabbed object */
	UPROPERTY()
	TArray<TWeakObjectPtr<APortal>> HeldThroughPortals;

	bool bIsPendingRelease = false;
	float ReleaseDelay = 0.f;
	
private:

	FVector GetDesiredGrabbedObjectLocation() const;

	void OnActorTeleported(TObjectPtr<ITeleportable> Actor, TObjectPtr<APortal> SourcePortal, TObjectPtr<APortal> TargetPortal);
	
	void OnGrabbedObjectTeleported(TObjectPtr<APortal> SourcePortal, TObjectPtr<APortal> TargetPortal);
	void OnOwnerCharacterTeleported(TObjectPtr<APortal> SourcePortal, TObjectPtr<APortal> TargetPortal);

	bool ShouldKeepHoldingObject() const;

	FQuat GetDesiredGrabbedObjectRotation();

};
