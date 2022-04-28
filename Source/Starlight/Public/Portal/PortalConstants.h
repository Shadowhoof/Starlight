#pragma once

class APortal;

/* Channel used by portal component to spawn portals */
#define ECC_Portal ECC_GameTraceChannel1
/* Object type of actor copies created by portals with type EPortalType::First */
#define ECC_FirstPortalCopy ECC_GameTraceChannel2
/* Object type of actor copies created by portals with type EPortalType::Second */
#define ECC_SecondPortalCopy ECC_GameTraceChannel3
	
DECLARE_LOG_CATEGORY_EXTERN(LogPortal, Log, All);

UENUM(BlueprintType)
enum class EPortalType : uint8
{
	First,
	Second
};

namespace PortalConstants
{
	const float ShootRange = 10000.f;
	const FVector Size = {0.f, 180.f, 250.f};
	const FVector HalfSize = Size / 2.f;
	const float OffsetFromSurface = 0.1f;
	const FVector BorderCollisionExtent = {50.f, 90.f, 125.f};

	/* materials */
	
	const FName CanBeCulledParam = "CanBeCulled";
	const FName CullPlaneCenterParam = "CullPlaneCenter";
	const FName CullPlaneNormalParam = "CullPlaneNormal";

	const float FloatTrue = 1.f;
	const float FloatFalse = 0.f;
}

inline EPortalType GetOtherPortalType(EPortalType PortalType)
{
	return PortalType == EPortalType::First ? EPortalType::Second : EPortalType::First;
}

/**
 * Returns object type of actor copy which will be created whenever a teleportable actor comes in contact
 * with portal of provided type.  
 */
inline ECollisionChannel GetCopyObjectType(EPortalType PortalType)
{
	return PortalType == EPortalType::First ? ECC_FirstPortalCopy : ECC_SecondPortalCopy;
}

/**
 * Returns object type of actor copy which will be created whenever a teleportable actor comes in contact
 * with portal of opposing type to the one provided.  
 */
inline ECollisionChannel GetOpposingCopyObjectType(EPortalType PortalType)
{
	return PortalType == EPortalType::First ? ECC_FirstPortalCopy : ECC_SecondPortalCopy;
}
