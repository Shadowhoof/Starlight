#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StarlightGameMode.generated.h"

class ITeleportable;
class APortal;

/**
 * Called when an actor is teleported via portal system.
 * @param Actor Actor that is being teleported
 * @param SourcePortal Portal that actor is being teleported from
 * @param TargetPortal Portal that actor is being teleported to
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FActorTeleported, TObjectPtr<ITeleportable> /* Actor */, TObjectPtr<APortal> /* SourcePortal */, TObjectPtr<APortal> /* TargetPortal */)

/**
 */
UCLASS()
class STARLIGHT_API AStarlightGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	AStarlightGameMode();

	FActorTeleported& OnActorTeleported();
	
private:
	
	FActorTeleported ActorTeleportedEvent;
};
