// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "TeleportableCopy.h"
#include "SkeletalTeleportableCopy.generated.h"

class UCapsuleComponent;


UCLASS()
class STARLIGHT_API ASkeletalTeleportableCopy : public ATeleportableCopy
{
	GENERATED_BODY()

public:
	
	ASkeletalTeleportableCopy();

	virtual void Initialize(TObjectPtr<ITeleportable> InParent, TObjectPtr<APortal> InOwnerPortal) override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	TWeakObjectPtr<USkeletalMeshComponent> ParentMeshComponent;

};
