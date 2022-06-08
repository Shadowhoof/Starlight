// Shadowhoof Games, 2022


#include "Portal/SkeletalTeleportableCopy.h"

#include "Components/CapsuleComponent.h"
#include "Core/StarlightCharacter.h"
#include "Portal/Teleportable.h"


ASkeletalTeleportableCopy::ASkeletalTeleportableCopy()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	SkeletalMeshComponent->SetupAttachment(CapsuleComponent);
	
	RootComponent = CapsuleComponent;
}

void ASkeletalTeleportableCopy::Initialize(TObjectPtr<ITeleportable> InParent, TObjectPtr<APortal> InOwnerPortal)
{
	Super::Initialize(InParent, InOwnerPortal);

	const AStarlightCharacter* Character = Cast<AStarlightCharacter>(InParent.Get());
	
	const UCapsuleComponent* ParentCapsule = Character->GetCapsuleComponent();
	float CapsuleRadius, CapsuleHalfHeight;
	ParentCapsule->GetScaledCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
	CapsuleComponent->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
	CapsuleComponent->SetCollisionResponseToChannels(ParentCapsule->GetCollisionResponseToChannels());

	ParentMeshComponent = Character->GetMesh();
	SkeletalMeshComponent->SetSkeletalMesh(ParentMeshComponent->SkeletalMesh);
	SkeletalMeshComponent->SetRelativeTransform(ParentMeshComponent->GetRelativeTransform());

	DisableCollisionWithPortal(CapsuleComponent);
	CreateDynamicInstances(SkeletalMeshComponent);
}

