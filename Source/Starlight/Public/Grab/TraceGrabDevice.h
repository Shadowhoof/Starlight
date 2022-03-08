// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GrabDevice.h"
#include "TraceGrabDevice.generated.h"

class AStarlightCharacter;

/**
 *  Grab device which uses ray cast to figure out what to grab
 */
UCLASS()
class STARLIGHT_API UTraceGrabDevice : public UGrabDevice
{
	GENERATED_BODY()

public:

	virtual bool TryGrabbing() override;

	void Initialize(TObjectPtr<AStarlightCharacter> Character);
	
private:

	UPROPERTY(Transient)
	TObjectPtr<AStarlightCharacter> PlayerCharacter = nullptr;

	virtual TObjectPtr<USceneComponent> GetComponentToAttachTo() const override;
	
};
