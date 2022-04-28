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
#include "Portal/TeleportableCopy.h"


static TAutoConsoleVariable CVarDebugDrawPortals(
                                                 TEXT("Portal.DebugDraw"),
                                                 false,
                                                 TEXT("Enables debug draw for portal-related stuff"));


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

	if (OtherPortal)
	{
		if (ActorsInPortalRange.Num() > 0)
		{
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

		for (const auto& Entry : TeleportableCopies)
		{
			ATeleportableCopy* Copy = Entry.Value;
			FTransform NewTransform = CalculateTransformForCopy(Copy->GetParent());
			Copy->SetActorTransform(NewTransform);
		}
	}
	

#if ENABLE_DRAW_DEBUG
	if (CVarDebugDrawPortals.GetValueOnGameThread())
	{
		const UWorld* World = GetWorld();
		const FTransform SurfaceSpaceTransform = {PortalSurface->GetActorQuat(), GetActorLocation()};

		const FVector TopLeft = SurfaceSpaceTransform.TransformPosition({0.f, -Extents.Y, Extents.Z});
		const FVector TopRight = SurfaceSpaceTransform.TransformPosition({0.f, Extents.Y, Extents.Z});
		const FVector BottomRight = SurfaceSpaceTransform.TransformPosition({0.f, Extents.Y, -Extents.Z});
		const FVector BottomLeft = SurfaceSpaceTransform.TransformPosition({0.f, -Extents.Y, -Extents.Z});
		DrawDebugLine(World, TopLeft, TopRight, FColor::Blue, false, - 1, 0, 1.f);
		DrawDebugLine(World, TopRight, BottomRight, FColor::Blue, false, - 1, 0, 1.f);
		DrawDebugLine(World, BottomRight, BottomLeft, FColor::Blue, false, - 1, 0, 1.f);
		DrawDebugLine(World, BottomLeft, TopLeft, FColor::Blue, false, - 1, 0, 1.f);

		const FVector Center = GetActorLocation();
		DrawDebugDirectionalArrow(World, Center, Center + GetActorUpVector() * PortalConstants::HalfSize.Z,
		                          20.f, FColor::Red, false, -1, 0, 1.f);
	}
#endif
}

void APortal::Initialize(const TObjectPtr<APortalSurface> Surface, FVector InLocalCoords, FVector InExtents,
                         EPortalType InPortalType)
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
	Extents = InExtents;

	// hide attached surface's mesh when capturing scene so it doesn't occlude the view
	if (UPrimitiveComponent* SurfaceCollisionComp = PortalSurface->GetAttachedSurfaceComponent())
	{
		SceneCaptureComponent->HideComponent(SurfaceCollisionComp);
	}
}

EPortalType APortal::GetPortalType() const
{
	return PortalType;
}

TObjectPtr<APortalSurface> APortal::GetPortalSurface() const
{
	return PortalSurface;
}

FVector APortal::GetLocalCoords() const
{
	return LocalCoords;
}

FVector APortal::GetExtents() const
{
	return Extents;
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
	if (!OtherPortal)
	{
		for (TScriptInterface<ITeleportable> Teleportable : ActorsInPortalRange)
		{
			Teleportable->OnOverlapWithPortalBegin(this);
		}
	}

	OtherPortal = Portal;

	for (const auto& Entry : TeleportableCopies)
	{
		ATeleportableCopy* Copy = Entry.Value;
		FTransform NewTransform = CalculateTransformForCopy(Copy->GetParent());
		Copy->SetActorTransform(NewTransform);
		Copy->UpdateCullingParams(Portal->GetActorLocation(), Portal->GetActorForwardVector());
	}
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

FVector APortal::TeleportLocation(const FVector& Location)
{
	if (!OtherPortal)
	{
		return Location;
	}

	const FVector RelativeLocation = BackfacingComponent->GetComponentTransform().InverseTransformPosition(Location);
	return OtherPortal->GetTransform().TransformPosition(RelativeLocation);
}

FRotator APortal::TeleportRotation(const FQuat& Quat)
{
	if (!OtherPortal)
	{
		return Quat.Rotator();
	}

	const FQuat RelativeQuat = BackfacingComponent->GetComponentQuat().Inverse() * Quat;
	return (OtherPortal->GetActorQuat() * RelativeQuat).Rotator();
}

FRotator APortal::TeleportRotation(const FRotator& Rotator)
{
	return TeleportRotation(FQuat(Rotator));
}

FVector APortal::TeleportVelocity(const FVector& Velocity)
{
	if (!OtherPortal)
	{
		return Velocity;
	}

	const FVector RelativeVelocity = BackfacingComponent->GetComponentQuat().UnrotateVector(Velocity);
	return OtherPortal->GetActorQuat().RotateVector(RelativeVelocity);
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
		if (OtherPortal)
		{
			TeleportableActor->OnOverlapWithPortalBegin(this);
			CreateTeleportableCopy(TeleportableActor);
		}
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

	if (OtherPortal)
	{
		TeleportableActor->OnOverlapWithPortalEnd(this);
		DeleteTeleportableCopy(Actor->GetUniqueID());
	}
	ActorsInPortalRange.Remove(TeleportableActor->GetTeleportableScriptInterface());
}

void APortal::TeleportActor(TObjectPtr<ITeleportable> TeleportingActor)
{
	OtherPortal->PrepareForActorTeleport(TeleportingActor);
	TeleportingActor->Teleport(this, OtherPortal);
	UE_LOG(LogPortal, Verbose, TEXT("Portal %s has teleported actor %s"), *GetName(),
	       *TeleportingActor->CastToTeleportableActor()->GetName());
}

bool APortal::ShouldTeleportActor(TObjectPtr<ITeleportable> TeleportingActor, const FVector PortalNormal) const
{
	const FVector ToActorDirection = (TeleportingActor->CastToTeleportableActor()->GetActorLocation() -
			GetActorLocation()).
		GetSafeNormal();
	return ToActorDirection.Dot(PortalNormal) < 0.f;
}

bool APortal::ShouldTeleportActor(TObjectPtr<ITeleportable> TeleportingActor) const
{
	return ShouldTeleportActor(TeleportingActor, GetActorRotation().Vector());
}

void APortal::CreateTeleportableCopy(TObjectPtr<ITeleportable> TeleportingActor)
{
	AActor* ParentActor = TeleportingActor->CastToTeleportableActor();
	const FTransform CopyTransform = CalculateTransformForCopy(ParentActor);
	ATeleportableCopy* Copy = TeleportingActor->CreatePortalCopy(CopyTransform, this, ParentActor);
	if (Copy)
	{
		TeleportableCopies.Add(ParentActor->GetUniqueID(), Copy);
		Copy->UpdateCullingParams(OtherPortal->GetActorLocation(), OtherPortal->GetActorForwardVector());
	}
}

void APortal::DeleteTeleportableCopy(int32 ParentObjectId)
{
	TObjectPtr<ATeleportableCopy>* CopyPtr = TeleportableCopies.Find(ParentObjectId);
	if (CopyPtr)
	{
		CopyPtr->Get()->Destroy();
		TeleportableCopies.Remove(ParentObjectId);
	}
}

FTransform APortal::CalculateTransformForCopy(TObjectPtr<const AActor> ParentActor) const
{
	const FTransform RelativeTransform = ParentActor->GetTransform().GetRelativeTransform(GetActorTransform());
	return RelativeTransform * OtherPortal->BackfacingComponent->GetComponentTransform();
}
