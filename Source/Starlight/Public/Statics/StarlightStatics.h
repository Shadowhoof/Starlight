// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "StarlightStatics.generated.h"

/**
 * 
 */
UCLASS()
class STARLIGHT_API UStarlightStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	static bool IsHMDActive();
	
};
