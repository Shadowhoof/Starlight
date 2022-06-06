// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "TeleportableCopy.h"
#include "GameFramework/Actor.h"
#include "StaticTeleportableCopy.generated.h"

class UCopyStaticMeshComponent;


UCLASS()
class STARLIGHT_API AStaticTeleportableCopy : public ATeleportableCopy
{
	GENERATED_BODY()

public:
	
	AStaticTeleportableCopy();

	virtual void Initialize(TObjectPtr<ITeleportable> InParent, TObjectPtr<APortal> InOwnerPortal) override;

	virtual void ResetVelocity() override;

	virtual void DispatchPhysicsCollisionHit(const FRigidBodyCollisionInfo& MyInfo,
	                                         const FRigidBodyCollisionInfo& OtherInfo,
	                                         const FCollisionImpactData& RigidCollisionData) override;

protected:
	
	UPROPERTY()
	TObjectPtr<UCopyStaticMeshComponent> StaticMeshComponent;
	
};
