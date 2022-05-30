// Shadowhoof Games, 2022


#include "Portal/Portal.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Core/StarlightConstants.h"
#include "Core/StarlightGameMode.h"
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
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PortalMesh->SetCollisionObjectType(ECC_PortalBody);
	PortalMesh->SetCollisionResponseToChannel(ECC_Portal, ECR_Ignore);
	PortalMesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	PortalMesh->SetCollisionResponseToChannel(ECC_GrabObstruction, ECR_Block);
	RootComponent = PortalMesh;

	BorderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BorderMesh"));
	BorderMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	BorderMesh->SetupAttachment(RootComponent);

	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCaptureComponent->bCaptureEveryFrame = true;
	SceneCaptureComponent->bEnableClipPlane = true;
	SceneCaptureComponent->SetupAttachment(RootComponent);

	CollisionBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBoxComponent"));
	CollisionBoxComponent->SetBoxExtent(PortalConstants::BorderCollisionExtent);
	CollisionBoxComponent->SetRelativeLocation(FVector(PortalConstants::BorderCollisionExtent.X, 0.f, 0.f));
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
			// FIXME - this should probably be in a different place, copy movement is all over the place (teleport here, physics in mesh component)
			FTransform NewTransform = CalculateTransformForCopy(Copy->GetParent());
			Copy->SetActorTransform(NewTransform);
			Copy->ResetVelocity();
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
                         EPortalType InPortalType, TObjectPtr<APortal> InOtherPortal)
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
	OtherPortal = InOtherPortal;

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

TObjectPtr<APortal> APortal::GetConnectedPortal() const
{
	return OtherPortal;
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

	UpdateSceneCaptureClipPlane();
}

void APortal::UpdateSceneCaptureTransform(const FTransform& RelativeTransform)
{
	SceneCaptureComponent->SetRelativeTransform(RelativeTransform);
}

FTransform APortal::GetBackfacingRelativeTransform(TObjectPtr<ACharacter> PlayerCharacter) const
{
	const FTransform ViewTransform = {PlayerCharacter->GetControlRotation(), PlayerCharacter->GetPawnViewLocation()};
	return ViewTransform.GetRelativeTransform(BackfacingComponent->GetComponentTransform());
}

void APortal::SetConnectedPortal(TObjectPtr<APortal> Portal)
{
	if (!Portal)
	{
		OtherPortal = nullptr;
		return;
	}
	
	if (!OtherPortal)
	{
		for (TScriptInterface<ITeleportable> Teleportable : ActorsInPortalRange)
		{
			Teleportable->OnOverlapWithPortalBegin(this);
		}
	}

	OtherPortal = Portal;
	UpdateSceneCaptureClipPlane();

	for (const auto& Entry : TeleportableCopies)
	{
		ATeleportableCopy* Copy = Entry.Value;
		FTransform NewTransform = CalculateTransformForCopy(Copy->GetParent());
		Copy->SetActorTransform(NewTransform);
		Copy->UpdateCullingParams(Portal->GetActorLocation(), Portal->GetActorForwardVector());
	}
}

void APortal::OnActorMoved(TObjectPtr<ITeleportable> Actor)
{
	if (ShouldTeleportActor(Actor))
	{
		TeleportActor(Actor);
	}
}

FVector APortal::TeleportLocation(const FVector& Location) const
{
	if (!OtherPortal)
	{
		return Location;
	}

	const FVector RelativeLocation = BackfacingComponent->GetComponentTransform().InverseTransformPosition(Location);
	return OtherPortal->GetTransform().TransformPosition(RelativeLocation);
}

FQuat APortal::TeleportRotation(const FQuat& Quat) const
{
	if (!OtherPortal)
	{
		return Quat;
	}

	const FQuat RelativeQuat = BackfacingComponent->GetComponentQuat().Inverse() * Quat;
	return OtherPortal->GetActorQuat() * RelativeQuat;
}

FRotator APortal::TeleportRotation(const FRotator& Rotator) const
{
	return TeleportRotation(FQuat(Rotator)).Rotator();
}

FVector APortal::TeleportVelocity(const FVector& Velocity) const
{
	if (!OtherPortal)
	{
		return Velocity;
	}

	const FVector RelativeVelocity = BackfacingComponent->GetComponentQuat().UnrotateVector(Velocity);
	return OtherPortal->GetActorQuat().RotateVector(RelativeVelocity);
}

TObjectPtr<ATeleportableCopy> APortal::RetrieveCopyForActor(TObjectPtr<AActor> Actor) const
{
	if (!Actor)
	{
		return nullptr;
	}

	const TObjectPtr<ATeleportableCopy>* CopyPtr = TeleportableCopies.Find(Actor->GetUniqueID());
	return CopyPtr ? *CopyPtr : nullptr;
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
	if (ITeleportable* TeleportableActor = Cast<ITeleportable>(Actor);
		TeleportableActor && !ActorsInPortalRange.Contains(TeleportableActor))
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
	if (ITeleportable* TeleportableActor = Cast<ITeleportable>(Actor))
	{
		if (OtherPortal)
		{
			TeleportableActor->OnOverlapWithPortalEnd(this);
			DeleteTeleportableCopy(Actor->GetUniqueID());
		}
		ActorsInPortalRange.Remove(TeleportableActor->GetTeleportableScriptInterface());
	}
}

void APortal::TeleportActor(TObjectPtr<ITeleportable> TeleportingActor)
{
	TeleportingActor->Teleport(this, OtherPortal);
	AStarlightGameMode* GameMode = GetWorld()->GetAuthGameMode<AStarlightGameMode>();
	if (GameMode)
	{
		GameMode->OnActorTeleported().Broadcast(TeleportingActor, this, GetConnectedPortal());
	}
	
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
	ATeleportableCopy* Copy = TeleportingActor->CreatePortalCopy(CopyTransform, this, OtherPortal, TeleportingActor);
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

void APortal::UpdateSceneCaptureClipPlane()
{
	if (!OtherPortal)
	{
		return;
	}

	SceneCaptureComponent->ClipPlaneBase = GetActorLocation();
	SceneCaptureComponent->ClipPlaneNormal = GetActorForwardVector();
}
