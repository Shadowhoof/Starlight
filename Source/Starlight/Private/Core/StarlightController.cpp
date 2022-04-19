// Shadowhoof Games, 2022


#include "Core/StarlightController.h"

#include "Core/StarlightCameraManager.h"


AStarlightController::AStarlightController()
{
	PlayerCameraManagerClass = AStarlightCameraManager::StaticClass();
}
