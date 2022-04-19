// 


#include "Core/StarlightGameMode.h"

#include "Core/StarlightCharacter.h"
#include "Core/StarlightController.h"

AStarlightGameMode::AStarlightGameMode()
{
	DefaultPawnClass = AStarlightCharacter::StaticClass();
	PlayerControllerClass = AStarlightController::StaticClass();
}
