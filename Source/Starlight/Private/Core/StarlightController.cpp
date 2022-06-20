// Shadowhoof Games, 2022


#include "Core/StarlightController.h"

#include "Core/StarlightCameraManager.h"

namespace Constants
{
	const float RollUpdateRate = 360.f;		/* Roll change per second if it's different from 0.0 */
}


AStarlightController::AStarlightController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	PlayerCameraManagerClass = AStarlightCameraManager::StaticClass();
}

void AStarlightController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!FMath::IsNearlyZero(ControlRotation.Roll))
	{
		ControlRotation.Roll = GetUpdatedAngle(DeltaSeconds, ControlRotation.Roll, Constants::RollUpdateRate);
		SetControlRotation(ControlRotation);
	}
}

float AStarlightController::GetUpdatedAngle(const float DeltaSeconds, float InitialAngle, const float UpdateRate) const
{
	InitialAngle = FRotator::NormalizeAxis(InitialAngle);
	const float DeltaAngle = DeltaSeconds * UpdateRate;
	return InitialAngle > 0.f
			   ? FMath::Max(InitialAngle - DeltaAngle, 0.f)
			   : FMath::Min(InitialAngle + DeltaAngle, 0.f);
}
