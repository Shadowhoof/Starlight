// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "StarlightController.generated.h"

UCLASS()
class STARLIGHT_API AStarlightController : public APlayerController
{
	GENERATED_BODY()

public:

	AStarlightController();

	virtual void Tick(float DeltaSeconds) override;

private:

	float GetUpdatedAngle(const float DeltaSeconds, float InitialAngle, const float UpdateRate) const;
	
};
