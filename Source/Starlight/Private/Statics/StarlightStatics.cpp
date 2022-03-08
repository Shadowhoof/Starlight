// Shadowhoof Games, 2022


#include "Statics/StarlightStatics.h"

#include "IXRTrackingSystem.h"

bool UStarlightStatics::IsHMDActive()
{
	return GEngine->XRSystem && GEngine->XRSystem->IsHeadTrackingAllowed();
}
