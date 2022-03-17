#pragma once

#define ECC_Portal ECC_GameTraceChannel1

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
}

inline EPortalType GetOtherPortalType(EPortalType PortalType)
{
	return PortalType == EPortalType::First ? EPortalType::Second : EPortalType::First;
}
