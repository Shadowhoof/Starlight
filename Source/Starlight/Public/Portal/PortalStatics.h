// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PortalConstants.h"
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
};
