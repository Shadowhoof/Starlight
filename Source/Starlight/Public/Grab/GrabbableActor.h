// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "Grabbable.h"
#include "GameFramework/Actor.h"
#include "GrabbableActor.generated.h"

UCLASS()
class STARLIGHT_API AGrabbableActor : public AActor, public IGrabbable
{
	GENERATED_BODY()

public:
	
	AGrabbableActor();

	virtual TObjectPtr<UPrimitiveComponent> GetAttachComponent() const override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StaticMesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

};
