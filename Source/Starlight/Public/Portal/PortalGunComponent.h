// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PortalConstants.h"
#include "PortalGunComponent.generated.h"


class APortalSurface;
class APortal;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STARLIGHT_API UPortalGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPortalGunComponent();

	void ShootPortal(EPortalType PortalType, const FVector& StartLocation, const FVector& Direction);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	TMap<EPortalType, TSubclassOf<APortal>> PortalClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	TMap<EPortalType, TObjectPtr<UTextureRenderTarget2D>> RenderTargets;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TMap<EPortalType, TObjectPtr<APortal>> ActivePortals;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;
	
	bool ValidatePortalLocation(EPortalType PortalType, const FHitResult& HitResult, TObjectPtr<APortalSurface> Surface,
	                            FVector& OutLocation, FVector& OutLocalCoords) const;

	bool IsOverlappingWithOtherPortal(EPortalType PortalType, TObjectPtr<APortalSurface> Surface,
	                                  const FVector& LocalCoords) const;
};
