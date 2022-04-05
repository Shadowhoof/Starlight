// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "Chaos/GeometryParticlesfwd.h"
#include "StarlightStatics.generated.h"


namespace Chaos
{
	class FIgnoreCollisionManager;
}


/**
 * 
 */
UCLASS()
class STARLIGHT_API UStarlightStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	static bool IsHMDActive();

	// Physics begin
	
	static void EnableCollisionBetween(TObjectPtr<UPrimitiveComponent> First, TObjectPtr<UPrimitiveComponent> Second);
	static void DisableCollisionBetween(TObjectPtr<UPrimitiveComponent> First, TObjectPtr<UPrimitiveComponent> Second);

	static bool IsPhysicsCollisionIgnored(TObjectPtr<UPrimitiveComponent> First, TObjectPtr<UPrimitiveComponent> Second);

	static Chaos::FIgnoreCollisionManager& GetIgnoreCollisionManager(TObjectPtr<UPrimitiveComponent> Component);

	static Chaos::FUniqueIdx GetPhysicsHandleID(TObjectPtr<UPrimitiveComponent> Component);
	
	// Physics end
};
