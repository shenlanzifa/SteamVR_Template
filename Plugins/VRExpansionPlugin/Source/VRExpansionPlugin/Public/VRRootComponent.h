// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "Components/ShapeComponent.h"
#include "VRRootComponent.generated.h"

//For UE4 Profiler ~ Stat Group
DECLARE_STATS_GROUP(TEXT("VRPhysicsUpdate"), STATGROUP_VRPhysics, STATCAT_Advanced);

// EXPERIMENTAL, don't use
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = VRExpansionLibrary)
class VREXPANSIONPLUGIN_API UVRRootComponent : public UShapeComponent
{
	GENERATED_UCLASS_BODY()

public:
	friend class FDrawCylinderSceneProxy;
	virtual class UBodySetup* GetBodySetup() override;

	void GenerateOffsetToWorld();

protected:
	virtual bool MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit = NULL, EMoveComponentFlags MoveFlags = MOVECOMP_NoFlags, ETeleportType Teleport = ETeleportType::None) override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;
	void SendPhysicsTransform(ETeleportType Teleport);

	const TArray<FOverlapInfo>* ConvertRotationOverlapsToCurrentOverlaps(TArray<FOverlapInfo>& OverlapsAtEndLocation, const TArray<FOverlapInfo>& CurrentOverlaps);
	const TArray<FOverlapInfo>* ConvertSweptOverlapsToCurrentOverlaps(
		TArray<FOverlapInfo>& OverlapsAtEndLocation, const TArray<FOverlapInfo>& SweptOverlaps, int32 SweptOverlapsIndex,
		const FVector& EndLocation, const FQuat& EndRotationQuat);

public:
	void UVRRootComponent::BeginPlay() override;

	bool IsLocallyControlled() const
	{
		// Epic used a check for a player controller to control has authority, however the controllers are always attached to a pawn
		// So this check would have always failed to work in the first place.....

		APawn* Owner = Cast<APawn>(GetOwner());

		if (!Owner)
		{
			//const APlayerController* Actor = Cast<APlayerController>(GetOwner());
			//if (!Actor)
			return false;

			//return Actor->IsLocalPlayerController();
		}

		return Owner->IsLocallyControlled();
	}

	// Whether to auto size the capsule collision to the height of the head.
	UPROPERTY(BlueprintReadWrite, Category = "VRExpansionLibrary")
	USceneComponent * TargetPrimitiveComponent;

	UPROPERTY(BlueprintReadOnly, Category = "VRExpansionLibrary")
	FTransform OffsetComponentToWorld;

	// Used to offset the collision (IE backwards from the player slightly.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRExpansionLibrary")
	FVector VRCapsuleOffset;

	FVector curCameraLoc;
	FRotator curCameraRot;

	FPrimitiveSceneProxy* CreateSceneProxy() override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
//};

protected:
	/**
	*	Half-height, from center of capsule to the end of top or bottom hemisphere.
	*	This cannot be less than CapsuleRadius.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = Shape, meta = (ClampMin = "0", UIMin = "0"))
		float CapsuleHalfHeight;

	/**
	*	Radius of cap hemispheres and center cylinder.
	*	This cannot be more than CapsuleHalfHeight.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = Shape, meta = (ClampMin = "0", UIMin = "0"))
		float CapsuleRadius;

protected:
	UPROPERTY()
		float CapsuleHeight_DEPRECATED;

public:
	/**
	* Change the capsule size. This is the unscaled size, before component scale is applied.
	* @param	InRadius : radius of end-cap hemispheres and center cylinder.
	* @param	InHalfHeight : half-height, from capsule center to end of top or bottom hemisphere.
	* @param	bUpdateOverlaps: if true and this shape is registered and collides, updates touching array for owner actor.
	*/
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		void SetCapsuleSize(float InRadius, float InHalfHeight, bool bUpdateOverlaps = true);

	/**
	* Set the capsule radius. This is the unscaled radius, before component scale is applied.
	* If this capsule collides, updates touching array for owner actor.
	* @param	Radius : radius of end-cap hemispheres and center cylinder.
	* @param	bUpdateOverlaps: if true and this shape is registered and collides, updates touching array for owner actor.
	*/
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		void SetCapsuleRadius(float Radius, bool bUpdateOverlaps = true);

	/**
	* Set the capsule half-height. This is the unscaled half-height, before component scale is applied.
	* If this capsule collides, updates touching array for owner actor.
	* @param	HalfHeight : half-height, from capsule center to end of top or bottom hemisphere.
	* @param	bUpdateOverlaps: if true and this shape is registered and collides, updates touching array for owner actor.
	*/
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		void SetCapsuleHalfHeight(float HalfHeight, bool bUpdateOverlaps = true);

	// Begin UObject interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	// End UObject interface

	// Begin USceneComponent interface
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual void CalcBoundingCylinder(float& CylinderRadius, float& CylinderHalfHeight) const override;
	// End USceneComponent interface

	// Begin UPrimitiveComponent interface.
	virtual bool IsZeroExtent() const override;
	virtual struct FCollisionShape GetCollisionShape(float Inflation = 0.0f) const override;
	virtual bool AreSymmetricRotations(const FQuat& A, const FQuat& B, const FVector& Scale3D) const override;
	// End UPrimitiveComponent interface.

	// Begin UShapeComponent interface
	virtual void UpdateBodySetup();// override;
	// End UShapeComponent interface

	// @return the capsule radius scaled by the component scale.
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		float GetScaledCapsuleRadius() const;

	// @return the capsule half height scaled by the component scale.
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		float GetScaledCapsuleHalfHeight() const;

	// @return the capsule radius and half height scaled by the component scale.
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		void GetScaledCapsuleSize(float& OutRadius, float& OutHalfHeight) const;


	// @return the capsule radius, ignoring component scaling.
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		float GetUnscaledCapsuleRadius() const;

	// @return the capsule half height, ignoring component scaling.
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		float GetUnscaledCapsuleHalfHeight() const;

	// @return the capsule radius and half height, ignoring component scaling.
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		void GetUnscaledCapsuleSize(float& OutRadius, float& OutHalfHeight) const;

	// Get the scale used by this shape. This is a uniform scale that is the minimum of any non-uniform scaling.
	// @return the scale used by this shape.
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
		float GetShapeScale() const;

	// Sets the capsule size without triggering a render or physics update. This is the preferred method when initializing a component in a class constructor.
	FORCEINLINE void InitCapsuleSize(float InRadius, float InHalfHeight)
	{
		CapsuleRadius = FMath::Max(0.f, InRadius);
		CapsuleHalfHeight = FMath::Max3(0.f, InHalfHeight, InRadius);
	}
};


// ----------------- INLINES ---------------

FORCEINLINE void UVRRootComponent::SetCapsuleRadius(float Radius, bool bUpdateOverlaps)
{
	SetCapsuleSize(Radius, GetUnscaledCapsuleHalfHeight(), bUpdateOverlaps);
}

FORCEINLINE void UVRRootComponent::SetCapsuleHalfHeight(float HalfHeight, bool bUpdateOverlaps)
{
	SetCapsuleSize(GetUnscaledCapsuleRadius(), HalfHeight, bUpdateOverlaps);
}

FORCEINLINE float UVRRootComponent::GetScaledCapsuleRadius() const
{
	return CapsuleRadius * GetShapeScale();
}

FORCEINLINE float UVRRootComponent::GetScaledCapsuleHalfHeight() const
{
	return CapsuleHalfHeight * GetShapeScale();
}

FORCEINLINE void UVRRootComponent::GetScaledCapsuleSize(float& OutRadius, float& OutHalfHeight) const
{
	const float Scale = GetShapeScale();
	OutRadius = CapsuleRadius * Scale;
	OutHalfHeight = CapsuleHalfHeight * Scale;
}


FORCEINLINE float UVRRootComponent::GetUnscaledCapsuleRadius() const
{
	return CapsuleRadius;
}

FORCEINLINE float UVRRootComponent::GetUnscaledCapsuleHalfHeight() const
{
	return CapsuleHalfHeight;
}

FORCEINLINE void UVRRootComponent::GetUnscaledCapsuleSize(float& OutRadius, float& OutHalfHeight) const
{
	OutRadius = CapsuleRadius;
	OutHalfHeight = CapsuleHalfHeight;
}

FORCEINLINE float UVRRootComponent::GetShapeScale() const
{
	return ComponentToWorld.GetMinimumAxisScale();
}