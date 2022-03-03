// 


#include "Core/StarlightGameMode.h"

#include "Core/StarlightCharacter.h"

AStarlightGameMode::AStarlightGameMode()
{
	DefaultPawnClass = AStarlightCharacter::StaticClass();
}
