#pragma once

class APortal;
	
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
	
	const FVector InnerCollisionExtent = {50.f, 90.f, 125.f};
	const FVector OuterCollisionExtent = {150.f, 270.f, 375.f};

	/* materials */
	
	const FName CanBeCulledParam = "CanBeCulled";
	const FName CullPlaneCenterParam = "CullPlaneCenter";
	const FName CullPlaneNormalParam = "CullPlaneNormal";

	const float FloatTrue = 1.f;
	const float FloatFalse = 0.f;
}
