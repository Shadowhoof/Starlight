// Shadowhoof Games, 2022


#include "Core/StarlightActor.h"

#include "Core/StarlightConstants.h"
#include "Portal/TeleportableCopy.h"
#include "Portal/Portal.h"

AStarlightActor::AStarlightActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComponent->SetCollisionResponseToChannel(ECC_FirstPortalCopy, ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_SecondPortalCopy, ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Portal, ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_GrabObstruction, ECR_Ignore);
	RootComponent = MeshComponent;
}

TObjectPtr<UPrimitiveComponent> AStarlightActor::GetComponentToGrab() const
{
	return MeshComponent;
}

void AStarlightActor::OnGrab()
{
	IGrabbable::OnGrab();
	bIsGrabbed = true;
}

void AStarlightActor::OnRelease()
{
	IGrabbable::OnRelease();
	bIsGrabbed = false;
}

bool AStarlightActor::IsGrabbed() const
{
	return bIsGrabbed;
}

void AStarlightActor::OnGrabbableMoved(const float Speed)
{
	GrabbedMovementSpeed = Speed;
}

void AStarlightActor::OnOverlapWithPortalBegin(TObjectPtr<APortal> Portal)
{
	ITeleportable::OnOverlapWithPortalBegin(Portal);

	OverlappingPortals.Add(Portal);
	UpdateMaterialParameters();
}

void AStarlightActor::OnOverlapWithPortalEnd(TObjectPtr<APortal> Portal)
{
	ITeleportable::OnOverlapWithPortalEnd(Portal);

	OverlappingPortals.Remove(Portal);
	UpdateMaterialParameters();
}

void AStarlightActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (OverlappingPortals.Num() > 1)
	{
		UpdateMaterialParameters();
	}
}

TObjectPtr<ATeleportableCopy> AStarlightActor::CreatePortalCopy(const FTransform& SpawnTransform,
                                                                TObjectPtr<APortal> OwnerPortal,
                                                                TObjectPtr<APortal> OtherPortal,
                                                                TObjectPtr<ITeleportable> ParentActor)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATeleportableCopy* Copy = GetWorld()->SpawnActor<ATeleportableCopy>(ATeleportableCopy::StaticClass(),
	                                                                    SpawnTransform, SpawnParams);
	Copy->Initialize(ParentActor, MeshComponent->GetStaticMesh(), OwnerPortal);
	return Copy;
}

void AStarlightActor::BeginPlay()
{
	Super::BeginPlay();

	DynamicMaterialInstance = MeshComponent->CreateDynamicMaterialInstance(0);
}

TObjectPtr<UPrimitiveComponent> AStarlightActor::GetCollisionComponent() const
{
	return MeshComponent;
}

void AStarlightActor::GetTeleportVelocity(FVector& LinearVelocity, FVector& AngularVelocity) const
{
	LinearVelocity = MeshComponent->GetPhysicsLinearVelocity();
	AngularVelocity = MeshComponent->GetPhysicsAngularVelocityInRadians();
}

void AStarlightActor::SetTeleportVelocity(const FVector& LinearVelocity, const FVector& AngularVelocity)
{
	MeshComponent->SetPhysicsLinearVelocity(LinearVelocity);
	MeshComponent->SetPhysicsAngularVelocityInRadians(AngularVelocity);
}

void AStarlightActor::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
                                bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (!bIsGrabbed || !OtherComp->IsSimulatingPhysics())
	{
		return;
	}

	const FVector Impulse = GrabbedMovementSpeed * -HitNormal;
	OtherComp->AddImpulseAtLocation(Impulse, HitLocation);
}

void AStarlightActor::UpdateMaterialParameters()
{
	const int32 PortalCount = OverlappingPortals.Num();
	if (PortalCount == 0)
	{
		DynamicMaterialInstance->SetScalarParameterValue(PortalConstants::CanBeCulledParam, PortalConstants::FloatFalse);
		return;
	}

	// We only consider the closest portal for cull plane because it's such a rare case when an object (and not its copies)
	// is in more than 1 portal at the same time and this case doesn't seem worth wasting time on
	APortal* ClosestPortal;
	if (PortalCount == 1)
	{
		ClosestPortal = OverlappingPortals[0];
	}
	else
	{
		const FVector Location = GetActorLocation();
		const float DistToFirst = FVector::DistSquared(Location, OverlappingPortals[0]->GetActorLocation());
		const float DistToSecond = FVector::DistSquared(Location, OverlappingPortals[1]->GetActorLocation());
		ClosestPortal = DistToFirst < DistToSecond ? OverlappingPortals[0] : OverlappingPortals[1];
	}

	DynamicMaterialInstance->SetScalarParameterValue(PortalConstants::CanBeCulledParam, PortalConstants::FloatTrue);
	DynamicMaterialInstance->SetVectorParameterValue(PortalConstants::CullPlaneCenterParam, ClosestPortal->GetActorLocation());
	DynamicMaterialInstance->SetVectorParameterValue(PortalConstants::CullPlaneNormalParam, ClosestPortal->GetActorForwardVector());
}
