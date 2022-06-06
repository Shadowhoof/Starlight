// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "Teleportable.h"
#include "GameFramework/Actor.h"
#include "TeleportableCopy.generated.h"

class APortal;
enum class EPortalType : uint8;


UCLASS(Abstract)
class STARLIGHT_API ATeleportableCopy : public AActor
{
	GENERATED_BODY()

public:
	
	ATeleportableCopy();

	/**
	 * Initializes created teleportable actor copy
	 * @param InParent Actor that served as a base for this copy
	 * @param InOwnerPortal Portal that created and owns this copy
	 */
	virtual void Initialize(TObjectPtr<ITeleportable> InParent, TObjectPtr<APortal> InOwnerPortal);

	/**
	 * Sets up culling parameters for dynamic materials
	 */
	void UpdateCullingParams(const FVector& CullPlaneCenter, const FVector& CullPlaneNormal);
	
	virtual TObjectPtr<AActor> GetParent() const;

	virtual void ResetVelocity();

	TWeakObjectPtr<APortal> GetOwnerPortal() const;
	
protected:

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterialInstances;

	UPROPERTY()
	TObjectPtr<AActor> ParentActor;
	
	UPROPERTY()
	TWeakObjectPtr<APortal> OwnerPortal;

protected:

	void DisableCollisionWithPortal(TObjectPtr<UPrimitiveComponent> CollisionComponent);
	
	void CreateDynamicInstances(TObjectPtr<UMeshComponent> MeshComponent);
	
};
