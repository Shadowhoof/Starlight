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

	static bool IsPhysicsCollisionIgnored(TObjectPtr<UPrimitiveComponent> First,
	                                      TObjectPtr<UPrimitiveComponent> Second);

	static Chaos::FIgnoreCollisionManager& GetIgnoreCollisionManager(TObjectPtr<UPrimitiveComponent> Component);

	static Chaos::FUniqueIdx GetPhysicsHandleID(TObjectPtr<UPrimitiveComponent> Component);

	// Physics end

	/**
	 * @brief Checks whether component will be inside blocking geometry at provided location and rotation. Calculates
	 * potential adjustment vector that will displace component from blocking collision.
	 * @see ComponentEncroachesBlockingGeometry_WithAdjustment
	 * @param Actor Owner of the component
	 * @param Component Component to check
	 * @param Location Location at which to check
	 * @param Rotation Component rotation to check
	 * @param IgnoredActors Actors to ignore during collision check
	 * @param Adjustment Proposed adjustment that will allow component to stay out of blocking geometry
	 * @return Is component inside blocking geometry
	 */
	static bool ComponentEncroachesBlockingGeometry(TObjectPtr<AActor> Actor, TObjectPtr<UPrimitiveComponent> Component, const FVector& Location,
	                                                const FRotator& Rotation,
	                                                const TArray<TObjectPtr<AActor>>& IgnoredActors,
	                                                FVector& Adjustment);
};
