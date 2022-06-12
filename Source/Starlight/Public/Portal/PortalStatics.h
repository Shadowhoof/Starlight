// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PortalConstants.h"
#include "Teleportable.h"
#include "Core/StarlightConstants.h"
#include "PortalStatics.generated.h"

class APortal;


USTRUCT()
struct STARLIGHT_API FPortalHitResult
{
	GENERATED_BODY()

	FHitResult HitResult;

	bool bIsThroughPortal;

	bool bIsBlockingHit;
};


/**
 * 
 */
UCLASS(Abstract)
class STARLIGHT_API UPortalStatics : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Transforms position using given transform but taking portals into account. If there is a portal between
	 * original location and transformed location then transformed location will be 'teleported' through the portal
	 * it encounters.
	 * @param WorldContextObject World context object for line trace
	 * @param Transform Transform used to calculate new location
	 * @param LocalPosition Position in local space to be transformed
	 * @param OutPosition Transformed position
	 * @return Whether portal was encountered
	 */
	static bool TransformPositionThroughPortal(TObjectPtr<UObject> WorldContextObject,
	                                           const FTransform& Transform,
	                                           const FVector& LocalPosition,
	                                           FVector& OutPosition);

	/**
	 * @brief Casts a line trace which will travel through any portals it encounters.
	 * @param World World in which to trace
	 * @param OutHitResult Result of the trace. Each crossed portal does a new trace, hit result of last of those
	 * traces will be stored here
	 * @param Start Start point of the trace
	 * @param End End point of the trace
	 * @param Channel Collision channel to use
	 * @param CrossedPortals Array of portal that were crossed by this trace
	 * @param QueryParams Collision query params
	 * @param ResponseParams Collision response params
	 * @return Whether trace resulted in a blocking hit. Collisions with portals don't count as blocking hits if they
	 * have a connected portal.
	 */
	static bool LineTraceThroughPortal(TObjectPtr<UWorld> World,
	                                   FHitResult& OutHitResult,
	                                   FVector Start,
	                                   FVector End,
	                                   ECollisionChannel Channel,
	                                   TArray<TObjectPtr<APortal>>* CrossedPortals = nullptr,
	                                   FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam,
	                                   FCollisionResponseParams ResponseParams = FCollisionResponseParams::DefaultResponseParam);

	static EPortalType GetOtherPortalType(EPortalType PortalType);

	/**
	 * Returns object type of actor copy which will be created whenever a teleportable actor comes in contact
	 * with portal of provided type.  
	 */
	static ECollisionChannel GetCopyObjectType(EPortalType PortalType);

	/**
	 * Returns object type of actor copy which will be created whenever a teleportable actor comes in contact
	 * with portal of opposing type to the one provided.  
	 */
	static ECollisionChannel GetOpposingCopyObjectType(EPortalType PortalType);

	/**
	 * Returns object type that a teleportable object would get on overlap with this portal type. Does not take into
	 * account if object is already overlapping with another portal.
	 */
	static ECollisionChannel GetInnerObjectTypeForPortalType(EPortalType PortalType);
	
	/* Returns object type appropriate for a teleportable object of provided type when it begins overlapping with a particular portal type. */
	static ECollisionChannel GetObjectTypeOnOverlapBegin(TObjectPtr<ITeleportable> Teleportable, EPortalType PortalType);

	/* Returns object type appropriate for a teleportable object of provided type when it stops overlapping with a particular portal type. */
	static ECollisionChannel GetObjectTypeOnOverlapEnd(TObjectPtr<ITeleportable> Teleportable, EPortalType PortalType);

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
	 * @param TargetPortal Destination portal
	 * @param ObjectType Object type to check collision against. If ECC_MAX is passed then object's current type will
	 * be used. ECC_MAX is the default value.
	 * @return Is component inside blocking geometry
	 */
	static bool ComponentEncroachesBlockingGeometryOnTeleport(TObjectPtr<AActor> Actor, TObjectPtr<UPrimitiveComponent> Component, const FVector& Location,
													const FRotator& Rotation, const TArray<TObjectPtr<AActor>>& IgnoredActors,
													FVector& Adjustment, TObjectPtr<APortal> TargetPortal, ECollisionChannel ObjectType = ECC_MAX);
};
