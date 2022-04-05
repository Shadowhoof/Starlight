// Shadowhoof Games, 2022


#include "Portal/Portal.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Portal/PortalConstants.h"
#include "Portal/PortalSurface.h"
#include "Portal/Teleportable.h"


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
	SceneCaptureComponent->bOverride_CustomNearClippingPlane = true;
	SceneCaptureComponent->SetupAttachment(RootComponent);

	CollisionBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBoxComponent"));
	CollisionBoxComponent->SetBoxExtent(PortalConstants::BorderCollisionExtent);
	CollisionBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBoxComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	CollisionBoxComponent->SetGenerateOverlapEvents(true);
	CollisionBoxComponent->SetupAttachment(RootComponent);

	BackfacingComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BackFacingComponent"));
	BackfacingComponent->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	BackfacingComponent->SetupAttachment(RootComponent);
}

void APortal::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!OtherPortal || ActorsInPortalRange.Num() == 0)
	{
		return;
	}

	const FVector PortalNormal = GetActorRotation().Vector();
	TArray<ITeleportable*> TeleportingActors;
	for (TScriptInterface<ITeleportable> ScriptInterface : ActorsInPortalRange)
	{
		ITeleportable* Teleportable = ScriptInterface.GetInterface();
		if (ShouldTeleportActor(Teleportable, PortalNormal))
		{
			TeleportingActors.Add(Teleportable);
		}
	}

	for (ITeleportable* Teleportable : TeleportingActors)
	{
		TeleportActor(Teleportable);
	}
}

void APortal::Initialize(const TObjectPtr<APortalSurface> Surface, FVector InLocalCoords, EPortalType InPortalType)
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
	PortalType = InPortalType;
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

void APortal::UpdateSceneCaptureTransform(const FTransform& RelativeTransform)
{
	SceneCaptureComponent->CustomNearClippingPlane = RelativeTransform.GetTranslation().Length();
	SceneCaptureComponent->SetRelativeTransform(RelativeTransform);
}

FTransform APortal::GetBackfacingRelativeTransform(TObjectPtr<ACharacter> PlayerCharacter) const
{
	const FTransform ViewTransform = {PlayerCharacter->GetControlRotation(), PlayerCharacter->GetPawnViewLocation()};
	return ViewTransform.GetRelativeTransform(BackfacingComponent->GetComponentTransform());
}

void APortal::SetConnectedPortal(TObjectPtr<APortal> Portal)
{
	OtherPortal = Portal;
}

void APortal::PrepareForActorTeleport(TObjectPtr<ITeleportable> TeleportingActor)
{
	if (OtherPortal->GetPortalSurface() != PortalSurface)
	{
		OnActorBeginOverlap(TeleportingActor->CastToTeleportableActor());
	}
}

void APortal::OnActorMoved(TObjectPtr<ITeleportable> Actor)
{
	if (ShouldTeleportActor(Actor))
	{
		TeleportActor(Actor);
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
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                         const FHitResult& SweepResult)
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
	ITeleportable* TeleportableActor = Cast<ITeleportable>(Actor);
	if (!TeleportableActor)
	{
		return;
	}

	if (!ActorsInPortalRange.Contains(TeleportableActor))
	{
		UE_LOG(LogPortal, Verbose, TEXT("Portal %s is now overlapping with %s"), *GetName(), *Actor->GetName());
		TeleportableActor->OnOverlapWithPortalBegin(this);
		ActorsInPortalRange.Add(TeleportableActor->GetTeleportableScriptInterface());
	}
}

void APortal::OnActorEndOverlap(TObjectPtr<AActor> Actor)
{
	ITeleportable* TeleportableActor = Cast<ITeleportable>(Actor);
	if (!TeleportableActor)
	{
		return;
	}

	UE_LOG(LogPortal, Verbose, TEXT("Portal %s is no longer overlapping with %s"), *GetName(), *Actor->GetName());
	TeleportableActor->OnOverlapWithPortalEnd(this);
	ActorsInPortalRange.Remove(TeleportableActor->GetTeleportableScriptInterface());
}

void APortal::TeleportActor(TObjectPtr<ITeleportable> TeleportingActor)
{
	const AActor* Actor = TeleportingActor->CastToTeleportableActor();
	
	const FRotator RelativeRotation = TeleportingActor->GetTeleportRotation() - BackfacingComponent->GetComponentRotation();
	const FRotator NewRotation = OtherPortal->GetActorRotation() + RelativeRotation;
	
	// TODO: this probably will have to be reworked slightly for wall-to-floor portals
	const FVector RelativeLocation = BackfacingComponent->GetComponentTransform().InverseTransformPosition(Actor->GetActorLocation());
	const FVector NewLocation = OtherPortal->GetTransform().TransformPosition(RelativeLocation);
	
	OtherPortal->PrepareForActorTeleport(TeleportingActor);
	TeleportingActor->Teleport(NewLocation, NewRotation);
	
	UE_LOG(LogPortal, Verbose, TEXT("Portal %s has teleported actor %s"), *GetName(), *Actor->GetName());
}

bool APortal::ShouldTeleportActor(TObjectPtr<ITeleportable> TeleportingActor, const FVector PortalNormal) const
{
	const FVector ToActorDirection = (TeleportingActor->CastToTeleportableActor()->GetActorLocation() - GetActorLocation()).
		GetSafeNormal();
	return ToActorDirection.Dot(PortalNormal) < 0.f;
}

bool APortal::ShouldTeleportActor(TObjectPtr<ITeleportable> TeleportingActor) const
{
	return ShouldTeleportActor(TeleportingActor, GetActorRotation().Vector());
}
