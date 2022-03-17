// Shadowhoof Games, 2022


#include "Portal/Portal.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Portal/PortalConstants.h"
#include "Portal/PortalSurface.h"


APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;
	
	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = PortalMesh;

	BorderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BorderMesh"));
	BorderMesh->SetupAttachment(RootComponent);

	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCaptureComponent->bCaptureEveryFrame = true;
	SceneCaptureComponent->SetupAttachment(RootComponent);

	CollisionBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBoxComponent"));
	CollisionBoxComponent->SetBoxExtent(PortalConstants::BorderCollisionExtent);
	CollisionBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBoxComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	CollisionBoxComponent->SetGenerateOverlapEvents(true);
	CollisionBoxComponent->SetupAttachment(RootComponent);

	BackFacingComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BackFacingComponent"));
	BackFacingComponent->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	BackFacingComponent->SetupAttachment(RootComponent);
}

void APortal::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!OtherPortal || ActorsInPortalRange.Num() == 0)
	{
		return;
	}

	const FVector SurfaceNormal = PortalSurface->GetActorRotation().Vector();
	TArray<TObjectPtr<AActor>> TeleportingActors;
	for (TObjectPtr<AActor> Actor : ActorsInPortalRange)
	{
		const FVector FromSurface = (Actor->GetActorLocation() - PortalSurface->GetActorLocation()).GetSafeNormal();
		if (FromSurface.Dot(SurfaceNormal) < 0.f)
		{
			TeleportingActors.Add(Actor);
		}
	}

	for (TObjectPtr<AActor> Actor : TeleportingActors)
	{
		TeleportActor(Actor);
	}
}

void APortal::Initialize(const TObjectPtr<APortalSurface> Surface, FVector InLocalCoords)
{
	if (PortalSurface)
	{
		UE_LOG(LogPortal, Error, TEXT("PortalSurface is already set for portal %s"), *GetName());
		return;
	}

	if (!Surface)
	{
		UE_LOG(LogPortal, Error, TEXT("PortalSurface is nullptr for portal %s"), *GetName());
		return;
	}

	PortalSurface = Surface;
	LocalCoords = InLocalCoords;
}

TObjectPtr<APortalSurface> APortal::GetPortalSurface() const
{
	return PortalSurface;
}

FVector APortal::GetLocalCoords() const
{
	return LocalCoords;
}

void APortal::SetRenderTargets(TObjectPtr<UTextureRenderTarget2D> ReadTarget,
	TObjectPtr<UTextureRenderTarget2D> WriteTarget)
{
	if (!ReadTarget || !WriteTarget)
	{
		UE_LOG(LogPortal, Error, TEXT("APortal::SetRenderTargets | Either ReadTarget or WriteTarget is nullptr"));
		return;
	}

	RenderTargetRead = ReadTarget;
	DynamicInstance->SetTextureParameterValue("PortalTexture", RenderTargetRead);
	PortalMesh->SetMaterial(0, DynamicInstance);
	
	RenderTargetWrite = WriteTarget;
	SceneCaptureComponent->TextureTarget = WriteTarget;
}

void APortal::UpdateSceneCaptureTransform(const FVector& RelativeLocation)
{
	SceneCaptureComponent->SetRelativeRotation(RelativeLocation.Rotation());
}

FVector APortal::GetRelativeLocationTo(TObjectPtr<ACharacter> PlayerCharacter) const
{
	return GetActorTransform().InverseTransformPosition(PlayerCharacter->GetPawnViewLocation());
}

void APortal::SetConnectedPortal(TObjectPtr<APortal> Portal)
{
	OtherPortal = Portal;
}

void APortal::PrepareForActorTeleport(TObjectPtr<AActor> TeleportingActor)
{
	if (OtherPortal->GetPortalSurface() != PortalSurface)
	{
		OnActorBeginOverlap(TeleportingActor);
	}
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	DynamicInstance = UMaterialInstanceDynamic::Create(PortalMesh->GetMaterial(0), this);

	CollisionBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnCollisionBoxStartOverlap);
	CollisionBoxComponent->OnComponentEndOverlap.AddDynamic(this, &APortal::OnCollisionBoxEndOverlap);
}

void APortal::OnCollisionBoxStartOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	OnActorBeginOverlap(OtherActor);
}

void APortal::OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	OnActorEndOverlap(OtherActor);
}

void APortal::OnActorBeginOverlap(TObjectPtr<AActor> Actor)
{
	if (!ActorsInPortalRange.Contains(Actor))
	{
		UE_LOG(LogPortal, Verbose, TEXT("Portal %s is now overlapping with %s"), *GetName(), *Actor->GetName());
		PortalSurface->SetCollisionEnabledForActor(Actor, false);
		ActorsInPortalRange.Add(Actor);
	}
}

void APortal::OnActorEndOverlap(TObjectPtr<AActor> Actor)
{
	UE_LOG(LogPortal, Verbose, TEXT("Portal %s is no longer overlapping with %s"), *GetName(), *Actor->GetName());
	PortalSurface->SetCollisionEnabledForActor(Actor, true);
	ActorsInPortalRange.Remove(Actor);
}

void APortal::TeleportActor(TObjectPtr<AActor> Actor)
{
	const TObjectPtr<APawn> Pawn = Cast<APawn>(Actor);
	const FRotator CurrentRotation = Pawn ? Pawn->GetControlRotation() : GetActorRotation();
	const FRotator RelativeRotation = CurrentRotation - BackFacingComponent->GetComponentRotation();
	const FRotator NewRotation = OtherPortal->GetActorRotation() + RelativeRotation;
	// TODO: this probably will have to be reworked slightly for wall-to-floor portals
	const float ZOffset = GetTransform().InverseTransformPosition(Actor->GetActorLocation()).Z;
	const FVector NewLocation = OtherPortal->GetActorLocation() + OtherPortal->GetActorUpVector() * ZOffset;
	OtherPortal->PrepareForActorTeleport(Actor);
	Actor->TeleportTo(NewLocation, NewRotation);
	UE_LOG(LogPortal, Verbose, TEXT("Portal %s has teleported actor %s"), *GetName(), *Actor->GetName());
}
