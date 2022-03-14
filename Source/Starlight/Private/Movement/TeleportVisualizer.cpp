// Shadowhoof Games, 2022


#include "Movement/TeleportVisualizer.h"


ATeleportVisualizer::ATeleportVisualizer()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = StaticMeshComponent;
}

void ATeleportVisualizer::UpdateVisualizerLocation(const FVector& NewLocation)
{
	SetActorLocation(NewLocation);
	FRotator NewRotation = (NewLocation - GetOwner()->GetActorLocation()).Rotation();
	NewRotation.Pitch = 0.f;
	SetActorRotation(NewRotation);
}

