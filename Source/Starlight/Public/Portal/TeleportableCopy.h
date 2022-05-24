// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "Teleportable.h"
#include "CopyStaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "TeleportableCopy.generated.h"

class APortal;
class AStarlightActor;
enum class EPortalType : uint8;


UCLASS()
class STARLIGHT_API ATeleportableCopy : public AActor
{
	GENERATED_BODY()

public:
	
	ATeleportableCopy();

	// TODO - support actors with skeletal meshes
	/**
	 * Initializes created teleportable actor copy
	 * @param InParent Actor that served as a base for this copy
	 * @param StaticMesh Parent actor's mesh
	 * @param OwnerPortal Portal that created and owns this copy
	 * @param OtherPortal Portal connected to the owner
	 */
	void Initialize(TObjectPtr<ITeleportable> InParent, TObjectPtr<UStaticMesh> StaticMesh,
					TObjectPtr<APortal> OwnerPortal, TObjectPtr<APortal> OtherPortal);

	/**
	 * Sets up culling parameters for dynamic material
	 */
	void UpdateCullingParams(const FVector& CullPlaneCenter, const FVector& CullPlaneNormal);
	
	TObjectPtr<AActor> GetParent() const;

	void ResetVelocity();

	virtual void DispatchPhysicsCollisionHit(const FRigidBodyCollisionInfo& MyInfo, const FRigidBodyCollisionInfo& OtherInfo, const FCollisionImpactData& RigidCollisionData) override;

protected:

	UPROPERTY()
	TObjectPtr<UCopyStaticMeshComponent> StaticMeshComponent;

	UPROPERTY()
	TScriptInterface<ITeleportable> ParentTeleportable;

	UPROPERTY()
	TObjectPtr<AActor> ParentActor;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterialInstance;
	
};
